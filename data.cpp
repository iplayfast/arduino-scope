#include <stdio.h>
#include <math.h>

int main(int argc,char *argv[])
{ //   y(t) = A \cdot \sin(\omega t + \varphi) 
double y1,y2;
double pi2 = 2 * 3.14;
	while(1)
	for(int  i = 0;i<pi2 * 1000;i++)
	{
	y1 = 256 * sin( i / 1000.0) + 256;
	y2 = 256 * cos( i / 1000.0 )+ 256;
	y1 = (y1<0) ? -y1 : y1;
	y2 = (y2<0) ? -y2 : y2;	
		printf("%d %d %d\n",i,(int)y1,(int)y2);
	}
}
