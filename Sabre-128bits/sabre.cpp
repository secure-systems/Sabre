


#include "dpf++/dpf.h"
#include "lowmc/lowmc.h"
#include <thread>
#include <iostream>
#include <deque>

#if(BLOCK_LEN == 128)
	#include "lowmc/constants_b128_r19_s32.h"
	#include "lowmc/recipe_b128_r19_s32.h"
#endif

#if(BLOCK_LEN == 256)
	#include "lowmc/constants_b256_r22_s17.h"
	#include "lowmc/recipe_b256_r22_s17.h"
#endif





#include "lowmc/streams.h"
#include "block.h"
#include <type_traits>  
#include <chrono>
using namespace std::chrono;



typedef unsigned char byte_t;



typedef std::conditional<BLOCK_LEN == 128,
                         __m128i,
                         __m256i>::type node_t;

 
constexpr size_t leaf_size = 64;
typedef std::array<__m128i, leaf_size> leaf_t;
//typedef std::array<block<row_t>, blocklen> bloc

typedef std::conditional<NDPFS == 128,
                         __m128i,
                         __m256i>::type row_t;

typedef lowmc::lowmc<blocklen,rounds, sboxes> prgkey_t;
 

 
typedef lowmc::bitsliced_lowmc<blocklen,rounds, sboxes, ndpfs> bitsliced_prgkey_t;
 
const size_t target = 2;
 
leaf_t val;// =  _mm_set1_epi64x(-1);

using namespace dpf;
using LowMC = lowmc::lowmc<blocklen,rounds, sboxes>;
using bitsliced_LowMC = lowmc::bitsliced_lowmc<blocklen,rounds, sboxes, ndpfs>;
using block_t = LowMC::block_t;

using instream     = LowMC::instream;
using outstream    = LowMC::outstream;
using rewindstream =  LowMC::rewindstream;
using basicstream  = LowMC::basicstream;
 

void print_first_col(block<__m256i> * inp, size_t col = 0)
{
  for(size_t i = 0; i < nitems; ++i)
  {
  	bool x = (inp[i]).bits[col];
  	printf("%d",x);
     //std::cout << (inp[i]).bits[col];
  }
 
 	printf("\n");
   //std::cout << std::endl;
}


void print_mX(block<__m128i> inp)
{
	for(size_t i = 0; i < 128; ++i)
  {
  	bool x = inp.bits[i];
  	printf("%d",x);
     //std::cout << (inp[i]).bits[col];
  }
 
 	printf("\n");
}
void print_mX(block<__m256i> inp)
{
	for(size_t i = 0; i < 256; ++i)
  {
  	bool x = inp.bits[i];
  	printf("%d",x);
     //std::cout << (inp[i]).bits[col];
  }
 
 	printf("\n");
}

void print_first_col(std::array<block<__m256i> , 128> inp, size_t col = 0)
{
  for(size_t i = 0; i < 128; ++i)
  {
  	bool x = (inp[i]).bits[col];
  	printf("%d",x);
     //std::cout << (inp[i]).bits[col];
  }
 
 	printf("\n");
   //std::cout << std::endl;
}


void gen_recipes(std::vector<intX> target_list, std::vector<uint8_t> recipe[bitlen + 1])
{
 
    for (int i = bitlen; i >= 0; --i)
     {
        target_list.erase(std::unique(std::begin(target_list), std::end(target_list)), std::end(target_list));
        for (size_t j = 0; j < target_list.size(); ++j)
        {
            if ((target_list[j] & 1) == 1)
            {
                recipe[i].push_back(0b01); // go right only
            }
            else if ((target_list[j] ^ target_list[j+1]) == 1)
            {
                recipe[i].push_back(0b11); // go both directions
                target_list[j++] >>= 1;
            }
            else
            {
                recipe[i].push_back(0b10); // go left only
            }
             target_list[j] >>= 1;
         }
      }

     
}


 
int main(int argc, char * argv[])
{

  AES_KEY aeskey;	
 	prgkey_t x;
 
 

	x.maska.shiftr(x.identity_len  - 1);
	x.maskb = x.maska >> 1;      /// mask for low-order bit in each s-box
	x.maskc = x.maska >> 2; 	/// mask for the all-but-the-highest-order bit in each s-box
	x.maskbc = x.maskb | x.maskc;
 
  
   
   

  constexpr size_t interval_len = nmailboxes - 1;

  printf("interval_len = %zu \n", interval_len);



 

 
 	std::vector<intX> target_list0;
   
 
  std::vector<intX> target_list1;
 
 
 	for(size_t j = 0; j < nmailboxes; ++j)
 	{

    intX r;
 		arc4random_buf(&r, sizeof(r));
  
    r = r % nitems;
    r = j;
    //r = j + 31;//r % nitems;
 		
    target_list0.push_back(r);
 
  

    target_list1.push_back(r);
 	} 
 
  
 
 

  // printf("Targets ...\n");
  // for(size_t j = 0; j < nmailboxes; ++j)
  // {
  //   printf("%zu. target = %lu -> %lu\n", j, target_list0[j], target_list0[j + 1]);
  //   if(j < nmailboxes - 1) assert(target_list0[j] < target_list0[j + 1]);
  //   printf("target = %lu\n", target_list0[j]);
  // } 
 std::sort(std::begin(target_list0), std::end(target_list0));
 std::sort(std::begin(target_list1), std::end(target_list1));
 // printf("target0 = %lu\n", target_list0[1] );
  auto [dpfkey0, dpfkey1] = dpf_key<leaf_t, node_t, prgkey_t>::gen(x, nitems, target_list0[325], val);

 






std::vector<uint8_t> recipe0[bitlen + 1];

 gen_recipes(target_list0, recipe0);

 
 
std::vector<uint8_t> recipe1[bitlen + 1];

 gen_recipes(target_list1, recipe1);

 
 
    auto start_eval0 = high_resolution_clock::now();
   
   uint8_t * _t0 = (uint8_t *) malloc ( sizeof(uint8_t) * (nmailboxes + 1));
   leaf_t * _output0 = (leaf_t *) std::aligned_alloc(sizeof(__m256i), (nmailboxes + 1) * sizeof(leaf_t));

   __evalinterval3(dpfkey0, 0, interval_len, _output0, _t0, recipe0, bitlen);
 

   printf("\n\n\n\n");

   auto stop_eval0 = high_resolution_clock::now(); 
   
   size_t duration_eval0 = duration_cast<milliseconds>(stop_eval0 - start_eval0).count();
   printf("duration_eval0 =  %zu\n", duration_eval0);
	
   uint8_t * _t1 = (uint8_t *) malloc ( sizeof(uint8_t) * (nmailboxes + 1));
  leaf_t * _output1 = (leaf_t *) std::aligned_alloc(sizeof(__m256i), (nmailboxes + 1) * sizeof(leaf_t));
 __evalinterval3(dpfkey1, 0, interval_len, _output1, _t1, recipe1, bitlen);

 

  for(size_t j = 0; j < interval_len; ++j)
  {
    if(_t0[j] != _t1[j]) printf("checking __evalinterval3 %zu \n", j);
  }

   



    bitsliced_prgkey_t bitsliced_prgkey;
  
  bitsliced_dpf_key<leaf_t, node_t, bitsliced_prgkey_t> dpf_array0;
  bitsliced_dpf_key<leaf_t, node_t, bitsliced_prgkey_t> dpf_array1;
  
  dpf_array0.finalizer = (leaf_t *) std::aligned_alloc(sizeof(__m256i), NDPFS * sizeof(leaf_t));
  dpf_array1.finalizer = (leaf_t *) std::aligned_alloc(sizeof(__m256i), NDPFS * sizeof(leaf_t));
  
  dpf_array0.nitems = nitems;
  dpf_array1.nitems = nitems;
  


    for(size_t j = 0; j < ndpfs; ++j)
      { 
        for(size_t j = 0; j < leaf_size; ++j) arc4random_buf(&val[j], sizeof(val[j]));
         
        auto [dpfkey0, dpfkey1] = dpf_key<leaf_t, node_t, prgkey_t>::gen(x, nitems, target, val);
 
        dpf_array0.root[j] = dpfkey0.root;       



        dpf_array0.cw.reserve(dpfkey0.depth(nitems));

        for(size_t d = 0; d < dpfkey0.depth(nitems); ++d)
        {
         dpf_array0.cw[d][j] = dpfkey0.cw[d];
        }

        
        dpf_array0.finalizer[j] = dpfkey0.finalizer;
          

        dpf_array1.root[j] = dpfkey1.root;
        
        dpf_array1.cw.reserve(dpfkey1.depth(nitems));

        for(size_t d = 0; d < dpfkey1.depth(nitems); ++d)
        {
         dpf_array1.cw[d][j] = dpfkey1.cw[d];
        }

         dpf_array1.finalizer[j] = dpfkey1.finalizer; 
      }

 
 	 auto start_eval1 = high_resolution_clock::now();
  leaf_t         * bitsliced_output0 = (leaf_t *) std::aligned_alloc(sizeof(__m256i),  nmailboxes * sizeof(leaf_t));
  block<__m256i> * flags0 = (block<__m256i> *) std::aligned_alloc(sizeof(__m256i), nmailboxes * sizeof(block<__m256i>));
  __evallist_bitsliced3(aeskey, bitsliced_prgkey, dpf_array0, flags0, 0, interval_len, bitsliced_output0, recipe0, bitlen, 0);
  auto stop_eval1 = high_resolution_clock::now(); 
   
   size_t duration_eval1 = duration_cast<milliseconds>(stop_eval1 - start_eval1).count();
   printf("duration_eval1 =  %zu\n", duration_eval1);

 
   std::vector<std::thread> evaluater(ncores); 
   auto start_eval_threads = high_resolution_clock::now(); 

    for(size_t j = 0; j < ncores; ++j)
    {   
      evaluater[j] = std::thread(__evallist_bitsliced3<leaf_t, node_t, bitsliced_prgkey_t, __m256i>, std::ref(aeskey), std::ref(bitsliced_prgkey),
                         std::ref(dpf_array0), flags0, 0, interval_len-1, bitsliced_output0, recipe0, bitlen, j);
    }

      for(size_t j = 0; j < ncores; ++j)
      {
        evaluater[j].join();
      }

  auto stop_eval_threads = high_resolution_clock::now();      
  size_t duration_eval_threads = duration_cast<milliseconds>(stop_eval_threads - start_eval_threads).count();
  printf("evaltime =  %f\n", ( double(duration_eval_threads)/(ncores * 256)));
   printf("evaltime =  %f\n", ( double(duration_eval_threads)));
  //  auto start_eval2 = high_resolution_clock::now();
  // leaf_t         * bitsliced_output2 = (leaf_t *) std::aligned_alloc(sizeof(__m256i),  nmailboxes * sizeof(leaf_t));
  // block<__m256i> * flags2 = (block<__m256i> *) std::aligned_alloc(sizeof(__m256i), nmailboxes * sizeof(block<__m256i>));
  // __evalinterval_bitsliced(aeskey, bitsliced_prgkey, dpf_array0, flags2, 0, interval_len, bitsliced_output2,0);
  //    auto stop_eval2 = high_resolution_clock::now(); 
   
  //  size_t duration_eval2 = duration_cast<milliseconds>(stop_eval2 - start_eval2).count();
  //  printf("duration_eval2 =  %zu\n", duration_eval2);

	return 0;
}
