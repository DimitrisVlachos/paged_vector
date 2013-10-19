
/*
	Paged-Vector implementation - Sample usage demo
	Autor	: Dimitris Vlachos
	Email	: DimitrisV22@gmail.com
	Git		: http://github.com/DimitrisVlachos
	Licence	: MIT
*/

#include <iostream>
#include <cstdio>
#include <vector> //Cmp vs vector
#include "paged_vector.hpp"

int main() {
    const uint32_t len = 16*1024*1024 + 7; //Try with odd len
#if 1
	const uint32_t page_size_in_bits = 25;
	const uint32_t page_reservation_per_reallocation = 4;
    paged_vector<char,page_size_in_bits,page_reservation_per_reallocation> v2;
    paged_vector<char,page_size_in_bits,page_reservation_per_reallocation> v;
#else
    std::vector<char> v;
    std::vector<char> v2;
#endif
	printf("Fill\n");
    for (uint32_t i = 0;i < len;++i)
        v2.push_back(i);

	printf("Assign\n");
    v=v2;
	printf("Cmp\n");
    for (uint32_t i = 0;i < len;++i) {
        if (v[i] != (char)i) {
            printf("Fail %u %u\n",v[i],i); break;
        }
    }
    printf("Ok\n");
    return 0;
}

/*

	CMP results : Fill , Assign , CMP
	len = 256*1024*1024 + 7;

	[paged_vector]
	Fill
	Assign
	Cmp
	Ok

	real	0m1.441s
	user	0m1.148s
	sys	0m0.292s



	[std::vector]
	Fill
	Assign
	Cmp
	Ok

	real	0m1.633s
	user	0m1.144s
	sys	0m0.488s




	CMP results : Fill , Assign 
	len = 256*1024*1024 + 7;

	[paged_vector]
	Fill
	Assign
	Cmp
	Ok

	real	0m1.007s
	user	0m0.724s
	sys	0m0.280s

	[std::vector]
	Fill
	Assign
	Cmp
	Ok

	real	0m1.408s
	user	0m0.840s
	sys	0m0.564s




	CMP results : Fill  
	len = 256*1024*1024 + 7;

	[paged_vector]
	Fill
	Ok

	real	0m0.635s
	user	0m0.508s
	sys	0m0.124s


	[std::vector]
	Fill
	Ok

	real	0m1.161s
	user	0m0.764s
	sys	0m0.396s



*/
