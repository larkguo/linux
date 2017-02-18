/*
JSON Demo -- by larkguo@gmail.com

1.Architecture:
  test	==API==>	json-c
  test	<==CallBack ==	json-c  (visit CB)
  
2.Requires:
  > json-c-0.12.1 ( https://github.com/json-c/json-c )
  
3.Compile:(assumed that json-c are installed in /usr/local)
  gcc -o test test.c -I/usr/local/include/json-c -L/usr/local/lib/ -ljson-c

4.Run:
  export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	./test
	
5.Valgrind:
	valgrind --error-limit=no --tool=memcheck --leak-check=full ./test
*/
#include <stdio.h>
#include "json.h"

int main(int argc,char **argv)
{
	struct json_object *jso = NULL,*jo1 = NULL,*jo2=NULL;
	char path[]="/obj2/subobj3";
	const char input[] = 
	"{\
			\"obj1\": 123,\
			\"obj2\": {\
					\"subobj1\": \"aaa\",\
					\"subobj2\": \"bbb\",\
					\"subobj3\": [ \"elem1\", \"elem2\", true ],\
			},\
			\"obj3\": 1.234,\
			\"obj4\": [ true, false, null ]\
	}";

	jso = json_tokener_parse(input);
	json_pointer_get(jso, path, &jo1);
	if(NULL != jo1){
		json_type type = json_object_get_type(jo1);
		if( json_type_array == type){
			int arrysize = 0, i = 0;
			arrysize = json_object_array_length(jo1);
			for(i=0; i < arrysize; i++){
				jo2 = json_object_array_get_idx(jo1, i);
				printf("\t%s[%d]=%s\n",path,i,json_object_to_json_string(jo2));
			}
		}
	}
	json_object_put(jso);
	
	return 0;
}
