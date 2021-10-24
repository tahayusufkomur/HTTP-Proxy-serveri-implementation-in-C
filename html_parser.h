int str_length(char * string)
{
	int length = 0;
	while (*string != '\0')
	{
		length++;
		string++;
	}


	return length;
}

int find_first_slash(char * string)
{
	int len = str_length(string);
	for(int i = 0; i < len; i++)
	{
		if(string[i] == '/')
			return i;
	}
	return -1;
}

char * str_concatenator(char * str, char ch, int type)
{
	int len = str_length(str);
	char *temp = (char*)malloc((len + 2) * sizeof(char));


	if (type == 1)
	{
		temp[0] = ch;
		for (int i = 1; i < len + 1; i++)
		{
			temp[i] = str[i - 1];
		}

	}
	int t = 0;
	if (type == 2)
	{
		for (int i = 0; i < len; i++)
		{
			temp[i] = str[i];
			t++;
		}
		temp[t] = ch;
		temp[t + 1] = '\0';

	}


	return temp;
}

int GET_CONTROL(char * html)
{
	int len= str_length(html);

	if (len>3)
	{	
		if (html[0] == 'G')
		{	
			if (html[1] == 'E')
			{
				if (html[2] == 'T')
				{
					return 1;
				}
			}
			 
		}
	}

	return -1;
}


int PUT_CONTROL(char * html)
{
	int len= str_length(html);

	if (len>3)
	{	
		if (html[0] == 'P')
		{	
			if (html[1] == 'U')
			{
				if (html[2] == 'T')
				{	
					printf("its PUT\n");
					return 1;
				}
			}
			 
		}
	}

	return -1;
}
// PUT, PATCH, and DELETE


int POST_CONTROL(char * html)
{
	int len= str_length(html);

	if (len>3)
	{	
		if (html[0] == 'P')
		{	
			if (html[1] == 'O')
			{
				if (html[2] == 'S')
				{
					if (html[3] == 'T')
					{
						printf("its POST\n");
						return 1;
					}
				}
			}
			 
		}
	}

	return -1;
}

int GET_SIZE(char * html)
{
	int len = str_length(html);
	char * temp = "";
	int i = find_first_slash(html);
	i++;
	for(i = i; i<len; i++)
	{	
		
		int htmlnum = html[i]-0;
		if( 47< htmlnum && htmlnum<57)
		{
			temp = str_concatenator(temp,html[i],2);
		}
		else
		{
			return atoi(temp);
		}
	}
return atoi(temp);
}

