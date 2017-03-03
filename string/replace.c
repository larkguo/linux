/*
string replace -- by larkguo@gmail.com

1.Compile:
	gcc  replace.c  -o replace

2.Run:
	# ./replace 

	begin[1,2,3,4,5,6,7] matched[,2,3,4,5,6,7] replaced[]
	begin[2,3,4,5,6,7] matched[,3,4,5,6,7] replaced[1]
	begin[3,4,5,6,7] matched[,4,5,6,7] replaced[12]
	begin[4,5,6,7] matched[,5,6,7] replaced[123]
	begin[5,6,7] matched[,6,7] replaced[1234]
	begin[6,7] matched[,7] replaced[12345]
	begin[7] matched[(null)] replaced[123456]
	replaced[1234567]
	......
	1,2,3,4,5,6,7_18:30-22:00 -> MTWHFAS_18:30-22:00

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUF_LEN (1024)
#define null_if_empty(s) (((s)!=NULL && (s)[0]!='\0')? (s) : NULL)

/**
\brief  replace all occurances of a substring (case sensitive)
        (dynamically allocates memory)
\param  in 	[IN]string before replace
\param  oldstr 	[IN]string keyword to be replaced
\param  newstr [IN] string to replace keyword with
\return string replaced
*/
static char *str_replace(const char *in, 
	const char *oldstr, const char *newstr) 
{
	char *replaced = NULL;	// string replaced 
	const char *begin = in; // string begin match 
	const char *matched = NULL; // string matched oldstr
	size_t len = 0;

	if( NULL == null_if_empty(in) 
		||NULL == null_if_empty(oldstr) 
		||NULL == newstr){
		return NULL;
	}
	replaced = strdup("");
	if( NULL == replaced )  return NULL;
	
	matched = strstr(begin, oldstr); 
	printf("\nbegin[%s] matched[%s] replaced[%s]\n",begin,matched,replaced);
	while ( NULL != matched ) {
		/* prepare space */
		len = (strlen(replaced)+(matched-begin)+strlen(newstr)+1)*sizeof(char);
		replaced = (char *)realloc(replaced,len);
		if( NULL == replaced ) 	return NULL;

		/* append before oldstr */
		replaced = strncat(replaced, begin, matched-begin);

		/* append newstr */
		replaced = strcat(replaced, newstr);

		/* begin next match, skip oldstr */
		begin = matched + strlen(oldstr);
		matched = strstr(begin, oldstr);
		printf("begin[%s] matched[%s] replaced[%s]\n",begin,matched,replaced);
	}

	/* after last oldstr */
	len = ( strlen(replaced)+strlen(begin)+1 ) * sizeof(char);
	replaced = (char *)realloc(replaced, len);
	if( NULL == replaced ) 	return NULL;
	replaced = strcat(replaced, begin);
	
	printf("replaced[%s]\n",replaced);
	return replaced;
}

static char *weekday_format(const char *days)
{
	char *str_in = NULL;
	char *str_out = NULL;
	int i;

	struct {
		char *oldstr;
		char *newstr;
	} pattern[] = {
		{ (char *)",",  (char *)"" }, //trim comma 
		{ (char *)"1",  (char *)"M" }, //Monday
		{ (char *)"2",  (char *)"T" }, //Tuesday
		{ (char *)"3",  (char *)"W" }, //Wednesday
		{ (char *)"4",  (char *)"H" }, //Thursday
		{ (char *)"5",  (char *)"F" }, //Friday
		{ (char *)"6",  (char *)"A" }, //Saturday
		{ (char *)"7",  (char *)"S" }, //Sunday
	};

	if(NULL == null_if_empty(days))
		return NULL;
	
	str_in = strdup(days);
	if(NULL == str_in) 
		return NULL;
	
	for (i=0; i<sizeof(pattern)/sizeof(pattern[0]); i++) {
		str_out = str_replace(str_in,pattern[i].oldstr,pattern[i].newstr);
		free(str_in);
		str_in = str_out;
	}
	return str_in;
}

#define TIME_STRING ("1,2,3,4,5,6,7_18:30-22:00")

int main()
{
	int ret = 0;
	char outbuf[MAX_BUF_LEN]={0};
	char week_days[MAX_BUF_LEN] = {0};
	char time_span[MAX_BUF_LEN] = {0};
	char *format_days = NULL;

	ret = sscanf(TIME_STRING,"%[^_]_%[^_]",week_days, time_span);
	if(2 == ret){
		format_days = weekday_format(week_days);
		if( NULL != format_days)	{
			snprintf(outbuf,sizeof(outbuf)-1,"%s_%s",format_days,time_span);

			printf("\n%s -> %s\n\n", TIME_STRING, outbuf);
			free(format_days);
			format_days = NULL;
		}
	}

	return 0;
}

