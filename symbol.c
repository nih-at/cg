

int
yourhash(char *s)
{
    int i, j, k;

    i = j = 0;
    
    while (*(s+i) != '\0') {
	j += (i+1)*(int)*(i+s);
	j %= TBLSIZE;
	i++;
    }

    k = j;

    j *= j;
    j %= TBLSIZE;
    j *= k;
    j %= TBLSIZE;

    return j;
    
}
