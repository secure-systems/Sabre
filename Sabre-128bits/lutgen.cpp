#include <cassert>
#include <iomanip>
#include <iostream>
#include<tuple>
#include <set>
#include <vector>

#define NROWS 128
#define NCOLS 32

int main(int argc, char * argv[])
{
	std::vector< std::tuple<int, int, int> > lut_recipe[NCOLS];
	uint8_t matrix8[NROWS*NCOLS]; // the matrix, cast to an array of uint8_t

	for (int j = 0; j < NCOLS; ++j)
	{
		// lexicographic (popcount, ordinal value) comparator
		auto cmp = [](uint8_t a, uint8_t b) { return std::make_pair(__builtin_popcount(a), a) < std::make_pair(__builtin_popcount(b), b); };
		std::set<uint8_t, decltype(cmp)> to_compute(cmp);
		for (int r = 0; r < NROWS; ++r)
		{
			auto i = matrix8[j*NCOLS+r];
			if (__builtin_popcount(i) > 1) to_compute.insert(i);
		}
		std::set<uint8_t, decltype(cmp)> did_compute({ 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }, cmp);

		while (!to_compute.empty())
		{
			bool progress;
			do
			{
				progress = false;
				for (auto it = to_compute.begin(); it != to_compute.end(); )
				{
					auto i = *it++;
					for (auto jj : did_compute)
					{
						if (did_compute.count(i^jj))
						{
							lut_recipe[j].push_back(std::make_tuple(i, jj, i^jj));
							to_compute.erase(i);
							did_compute.insert(i);
							progress = true;
							break;
						}
					}
				}
			} while (progress);

			if (!to_compute.empty())
			{
				auto i = *to_compute.begin();
				for (uint8_t subset=(i-1)&i; subset != 0; subset=(subset-1)&i)
				{
						auto complement = i-subset;
						if (did_compute.count(subset))
						{
							uint8_t curr = 1U << __builtin_ctz(subset);
							// compute complement the naive way
							while (subset-curr)
							{
								uint8_t next = 1U << __builtin_ctz(subset-curr);
								lut_recipe[j].push_back(std::make_tuple(curr+next, curr, next));
								curr+=next;
								did_compute.insert(curr);
							}
							to_compute.erase(i);
							did_compute.insert(i);
							continue;
						}
				}
			}
		}

	}


	for (size_t i = 0; i < NCOLS; ++i)
	{
		std::cout << "-> " << i << " ---> " << lut_recipe[i].size() << std::endl;	
	}

  	std::cout <<  "std::vector< std::tuple<int, int, int> > lut_recipe[NCOLS] " << std::endl;
  	std::cout <<  "for(size_t cols = 0; cols < NCOLS; ++cols)\n";
  	std::cout <<  "{\n";
  	std::cout <<  "	lut_recipe[cols].resize(lut_recipe[i].size());\n";
  	std::cout << "}\n";


  	std::cout <<  "for(size_t cols = 0; cols < NCOLS; ++cols)\n";
  	std::cout <<  "{\n";
  	std::cout <<  "for(size_t j = 0; j < NCOLS; ++j)\n";
  	std::cout <<  "{\n";
  	
  	for(size_t cols = 0; cols < NCOLS; ++cols)
  	{
	  	for(size_t i = 0; i < lut_recipe[cols].size(); ++i)
	  	{
	  		lut_recipe[cols][i] = std::make_tuple(std::get<0>(lut_recipe[cols][i]), std::get<1>(lut_recipe[cols][i]), std::get<2>(lut_recipe[cols][i]));
	  	}
  	}
  	std::cout <<  "	lut_recipe[cols][j] = "  << std::get<0>(lut_recipe[22][0])  << " ;\n";

	std::cout << " }\n";  	
  	std::cout << "}\n";



	return 0;
}

