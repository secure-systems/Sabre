#include "dpf++/dpf.h"
#include "lowmc/lowmc.h"
#include <iostream>
#include "lowmc/constants_b128_r144_s2.h"
#include "lowmc/streams.h"
#include "lowmc/block.h"

typedef unsigned char byte_t;
typedef bool leaf_t;
typedef __m128i node_t;
typedef lowmc::lowmc<128,144,2> prgkey_t;
 
const size_t nitems = 1ULL << 20;  
const size_t target = 22;
const leaf_t val =  1; //_mm256_set1_epi8(0x01);

using namespace dpf;
using LowMC = lowmc::lowmc<128,144,2>;
using block_t = LowMC::block_t;

using instream     = LowMC::instream;
using outstream    = LowMC::outstream;
using rewindstream =  LowMC::rewindstream;
 using basicstream  = LowMC::basicstream;
 
void multiply_mpc_p2(outstream * p0, outstream * p1)
{
	block_t beta0, beta1;
	bool alpha0, alpha1;

	arc4random_buf(&beta0, sizeof(block_t));
	arc4random_buf(&beta1, sizeof(block_t));

 	printf("beta0 = %lld\n", beta0.mX[0]);	
 	printf("beta1 = %lld\n", beta1.mX[0]);
 	
 	alpha0 = rand();
 	alpha1 = rand();

 	block_t rand;

 	arc4random_buf(&rand, sizeof(block_t));
 	
	block_t gamma0 = xor_if(rand, beta0, alpha1);
	block_t gamma1 = xor_if(rand, beta1, alpha0);

 	printf("---> %lld\n", gamma0.mX[0]);	
 	printf("---> %lld\n", gamma1.mX[0]);
    
    *p0 << gamma0;
    *p1 << gamma1;  
}


void multiply_mpc_p0(instream * p2)
{
	 
	 block_t gamma;

	 *p2 >> gamma;

	 printf("---> %lld\n", gamma.mX[0]);	
}
 
 void multiply_mpc_p1(instream * p2)
{
	 
	 block_t gamma;

	 *p2 >> gamma;

	 printf("---> %lld\n", gamma.mX[0]);	
}

void conditional_swap()
{

}


void root_layer(LowMC prgkey, const dpf_key<leaf_t, node_t, prgkey_t>& dpfkey0, const dpf_key<leaf_t, node_t, prgkey_t>& dpfkey1)
{
   	size_t cur_depth = 0;

    const block_t seed0 = dpfkey0.root; 
    const block_t seed1 = dpfkey1.root;

    auto & cw0 = dpfkey0.cw[cur_depth];
    uint8_t cw_t0[2] = { get_lsb(cw0, 0b01), get_lsb(cw0, 0b10) };

    
    auto & cw1 = dpfkey1.cw[cur_depth];
    uint8_t cw_t1[2] = { get_lsb(cw1, 0b01), get_lsb(cw1, 0b10) };
    
    bool b0 = get_lsb(dpfkey0.root);
    bool b1 = get_lsb(dpfkey1.root);

    block_t s0[2], s1[2];    uint8_t t0[2], t1[2];

    block_t child0[2];

    expand(prgkey, seed0, child0, t0);
  
    // s0[L] and s0[R] are P0's left and right children after the correction word is applied
    s0[L] = clear_lsb(xor_if(child0[L], cw0, !b0), 0b11); 
    s0[R] = clear_lsb(xor_if(child0[R], cw0, !b0), 0b11); 
   
    block_t child1[2];
    
    
    expand(prgkey, seed1,  child1 , t1);

    s1[L] = clear_lsb(xor_if(child1[L], cw0, !b1), 0b11);
    s1[R] = clear_lsb(xor_if(child1[R], cw0, !b1), 0b11);


    block_t s0_L[2]; 
    block_t s0_R[2];  

    //s0_L[0] = P0rand.next_block();
    arc4random_buf(&s0_L[0], sizeof(__m128i));
    s0_L[1] = s0_L[0] ^ s0[L];

    //s0_R[0] = P0rand.next_block(); 
	arc4random_buf(&s0_R[0], sizeof(__m128i));
    s0_R[1] = s0_R[0] ^ s0[R];
  

    block_t s1_L[2]; 
    block_t s1_R[2];
   
    
    //s1_L[0] = P1rand.next_block();   
    arc4random_buf(&s1_L[0], sizeof(__m128i)); 
    s1_L[1] = s1_L[0] ^ s1[L];
  
    //s1_R[0] = P1rand.next_block();
    arc4random_buf(&s1_R[0], sizeof(__m128i));
    s1_R[1] = s1_R[0] ^ s1[R];

}

std::array<__m128i, 256> seed0_array;
std::array<__m128i, 256> seed1_array;

int main(int argc, char * argv[])
{
	// using LowMC = lowmc::lowmc<128,144,2>;

	// using block_t = LowMC::block_t;

	lowmc::randomness rand0(1024*sizeof(__m128));
	lowmc::randomness rand1(1024*sizeof(__m128));
	lowmc::randomness rand2(1024*sizeof(__m128));

	LowMC x;
	block_t msg1(31337);
	auto c = x.encrypt(msg1);

	LowMC::basicstream p20, p21;
	auto r0 = rand0, r1 = rand1, r2 = rand2;
	x.encrypt2_p2(&p20, &p21, r0, r1, r2);

	LowMC::basicstream p01, p10;
	block_t share0 = 0;
	arc4random_buf(&share0, sizeof(share0));
	block_t share1 = msg1 ^ share0;

	auto c0 = std::async(std::launch::async, &LowMC::encrypt2_p0p1, x, std::ref(share0), std::ref(rand0), &p10, &p20, &p01, 0);
	auto c1 = std::async(std::launch::async, &LowMC::encrypt2_p0p1, x, std::ref(share1), std::ref(rand1), &p01, &p21, &p10, 1);
	auto C = c0.get() ^ c1.get();

	LowMC::rewindstream s10, s01, s20, s21;
	auto r_ = rand0;
	auto [d0, d1] = x.encrypt3_proof(share0, share1, rand0, rand1, rand2, &s01, &s10, &s20, &s21);
	auto D = x.encrypt3_verify(share0, s01.sha256_digest(), &s10, &s20, r_, 0);

	printf("%x\n", D);
	printf("0x%016llx =?= 0x%016llx =?= 0x%016llx\n", ((__m128i)c)[0], ((__m128i)C)[0], ((__m128i)d0^(__m128i)d1)[0]);
 
 	auto [dpfkey0, dpfkey1] = dpf_key<leaf_t, node_t, prgkey_t>::gen(x, nitems, target, val);
	
 	root_layer(x, dpfkey0, dpfkey1);
 	
 	multiply_mpc_p2(&p20, &p21);
 	multiply_mpc_p0(&p20);
 	multiply_mpc_p1(&p21);

 	leaf_t * output0 = (leaf_t *)std::aligned_alloc(sizeof(node_t), dpfkey0.full_bytes());
    dpfkey0.evalfull(output0);
	
	return 0;
}
