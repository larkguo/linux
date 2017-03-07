
/*
 key&value regex   -- by larkguo@gmail.com


1.Compile:
	gcc regex.c -o regex
	
2.Run:
	# ./regex 
	string(abc=123) pattern(^([a-z][a-z0-9_]*)=([a-z0-9_]*)$)
	match:
	        array[0] 0-7 is 'abc=123'
	        array[1] 0-3 is 'abc'
	        array[2] 4-7 is '123'
	key(abc)=value(123)
*/

#include <stdio.h>
#include <string.h>
#include <regex.h>

int main(int argc,char *argv[])
{
	char *string = "abc=123";
	char *pattern = "^([a-z][a-z0-9_]*)=([a-z0-9_]*)$" ;
	size_t narray = 3;
	size_t offset = 0;
	regex_t reg;
	regmatch_t matchs[narray]; 
	int ret = 0;

	printf("string(%s) pattern(%s)\n",string,pattern);
	ret = regcomp(&reg, pattern, REG_EXTENDED|REG_ICASE);
	if( 0 != ret){
		printf("regcomp failed %d\n",ret);
		return -1;
	}

	ret = regexec(&reg, string, narray, matchs, 0);
	if( 0 == ret ){ /* match */
		char array[narray][strlen(string) + 1];

		printf("match:\n");
		for( offset = 0; offset < narray; offset++){
			char copy[strlen(string) + 1] ;
			strcpy(copy, string);
			copy[matchs[offset].rm_eo] = 0;
			strncpy(array[offset],copy+matchs[offset].rm_so,sizeof(array[offset])-1);
			printf("\tarray[%u] %u-%u is '%s'\n",offset,
				matchs[offset].rm_so,matchs[offset].rm_eo,copy+matchs[offset].rm_so);
		}
		printf("key(%s)=value(%s)\n",array[1],array[2]);
	}else{ /* no match */
		printf("regexec failed %d\n",ret);
	}
	regfree(&reg);
	return 0;
}
