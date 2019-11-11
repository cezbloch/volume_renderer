#include <iostream>
#include <fstream>
#include <sstream>	//for istringstream
#include <algorithm>
#include <time.h>
#include <conio.h>
#include "vec2.h"
#include "vec3.h"


using namespace std;

class tex_type
{
public:
vec3usi gradients;
unsigned short int size;
};

class grad_float
{
public:
vec3si gradients;
short int data;
};

grad_float *all_data;
unsigned char *sphere_data;
vec3c *multi_data;
vec3si *gradients;
tex_type *bubble_data;
bool *mask_data;
vec3i size;
int max_val,coef,zero_val,max_bubble_size;


void write_data(int x,int y,int z,grad_float tmp)
{all_data[x+y*size.x+z*(size.x*size.y)]=tmp;}

void write_char_data(int x,int y,int z,unsigned char tmp)
{sphere_data[x+y*size.x+z*(size.x*size.y)]=tmp;}

void write_multi_data(int x,int y,int z,vec3c tmp)
{multi_data[x+y*size.x+z*(size.x*size.y)]=tmp;}

void write_grad_data(int x,int y,int z,vec3si tmp)
{gradients[x+y*size.x+z*(size.x*size.y)]=tmp;}

int rand_int(int max)
{return (int)((float)(max*rand())/(float)RAND_MAX);}

bool overwrite(vec3i center,int radius)
{
int x,y,z;
//check if there is already something written where we plan to write
for(z=center.z-radius;z<center.z+radius;z++)
	for(y=center.y-radius;y<center.y+radius;y++)
		for(x=center.x-radius;x<center.x+radius;x++)
			if(mask_data[x+y*size.x+z*(size.x*size.y)]==true)return true;
return false;
}

void write_bubble(vec3i center,int radius)
{
int x,y,z;
vec3i diff,curr;
vec3usi grad;
double distance;
for(z=center.z-radius;z<center.z+radius;z++)
	for(y=center.y-radius;y<center.y+radius;y++)
		for(x=center.x-radius;x<center.x+radius;x++)
		{
			curr=vec3i(x,y,z);
			diff=curr-center;
			distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
			if(distance<=radius)
			{
				//if(distance!=0)
				{
					grad.x=unsigned short int(zero_val+zero_val*diff.x/distance);
					grad.y=unsigned short int(zero_val+zero_val*diff.y/distance);
					grad.z=unsigned short int(zero_val+zero_val*diff.z/distance);
				}
				mask_data[x+y*size.x+z*(size.x*size.y)]=true;
				bubble_data[x+y*size.x+z*(size.x*size.y)].size=unsigned short int(max_val*distance/(float)max_bubble_size);
				//bubble_data[x+y*size.x+z*(size.x*size.y)].size=zero_val;
				//cout<<unsigned short int(max_val*distance/(float)max_bubble_size)<<endl;
				bubble_data[x+y*size.x+z*(size.x*size.y)].gradients=grad;
			}			
		}
}

int main(int argc,char* argv[])
{
if (argc<4){cout<<"program usage: dataset[1] outname[2] sizes[3] mode[4] multi_dim[5]"<<endl
<<"mode attribute:"<<endl
<<"s - produces short int sphere with short int normals"<<endl
<<"c - produces char sphere only"<<endl
<<"t - produces short int texture,3 components are gradients of bubbles inside,4th is radius of bubble"<<endl
<<"m - produces multi variate char data with gradients"<<endl<<endl
<<"example: dataset kula.raw 256 b"<<endl
<<"this will produce 256x256x256 sphere dataset containing char values (from 0 to 255)"<<endl
<<"if using mode 'm' then additional parameter is needed,that tell now many spheres with extra parameter will "
<<"be create on each axis! nr_spheres^3 "<<endl;exit(1);}

size.x=atoi(argv[2]);
size.y=atoi(argv[2]);
size.z=atoi(argv[2]);
int nr_balls=0;
char mode;
mode=argv[3][0];
if((mode=='m'||mode=='t')&&argc!=5)cout<<"too little arguments!check program usage"<<endl;
if(mode=='m'||mode=='t')nr_balls=atoi(argv[4]);
cout<<"nr balls:"<<nr_balls<<endl;
int z,x,y;
int i,j,k;
long int written=0;
//maximum distance depending on volume size and written in proper format
//divide by 4 cause of 4 components in grad_float, and by 2 cause we consider distance from center
int length,max_dst=size.x/2-1;
//int max_val=pow(255.0f,2)/2-1;
double distance;
string name,grad_name;
length=size.x*size.y*size.z;
cout<<size.x<<" "<<size.y<<" "<<size.z<<endl;
name=argv[1];
ofstream output(name.c_str(), ios::out | ios::binary);
grad_name=name+".grad";
ofstream grad_out(grad_name.c_str(), ios::out | ios::binary);
	vec3i diff,curr,center;
	center.x=size.x/2-1;
	center.y=size.y/2-1;
	center.z=size.z/2-1;


if(mode=='c')
	{
	cout<<"writing unsigned chars only"<<endl;
	char temp;
	max_val=100;
	sphere_data=new unsigned char[length];
	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
				curr=vec3i(x,y,z);
				diff=curr-center;
				distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
				if(distance<max_dst&&distance>max_dst*0.6)
				{
					//temp.data=short int (abs(double(0.8*max_dst)-distance)*max_val*10);
					temp=unsigned char(max_val);
					written++;
				}
				else
				{
					temp=0;
				}
				write_char_data(x,y,z,temp);
				}
	output.write((char*)&sphere_data[0],sizeof(unsigned char)*length);
	cout<<written<<" voxels written in. Max_val "<<max_val<<endl;
	}



if(mode=='t')
{
	int radius,nr_bubbles=0;
	bool next=true;
	max_val=65536-1;
	coef=256/2;
	zero_val=65536/2-1;

	cout<<"allocating memory "<<zero_val<<endl;
	bubble_data=new tex_type[length];
	mask_data=new bool[length];
	//memset(bubble_data,0,sizeof(bubble_data));    
	for(i=0;i<length;i++)
	{
		bubble_data[i].gradients=vec3usi(zero_val,zero_val,zero_val);
		bubble_data[i].size=0;
	}
	memset(mask_data,0,sizeof(mask_data));    

	cout<<"producing bubbles"<<endl;
	srand(time(NULL));
	while(1)
	{
	//radius=5+rand_int(10);
	radius=nr_balls;
	max_bubble_size=nr_balls;
	//cout<<radius<<" "<<endl;
	//make sure bubbles will fit entirely in the volume (center will be at radius distance from the edge
	center.x=radius+rand_int(size.x-2*radius);
	center.y=radius+rand_int(size.y-2*radius);
	center.z=radius+rand_int(size.z-2*radius);

	if(!overwrite(center,radius))//if it will overwrite existing sphere try somewhere else
		{
		write_bubble(center,radius);
		nr_bubbles++;
		cout<<"bubble nr "<<nr_bubbles<<" of size "<<radius<<" written into volume.Trying to fit next bubble..."<<endl;
		next=true;
		}
	//if(nr_bubbles>=nr_balls)break;
	if(kbhit())break;
	}	
	cout<<"writing volume of size "<<sizeof(tex_type)*length<<" to disc"<<endl;
	output.write((char*)&bubble_data[0],sizeof(tex_type)*length);
}
max_val=32768-1;
coef=256/2;
zero_val=32768/2-1;

if(mode=='m')
	{
	cout<<"producing multivariate char data"<<endl;
	vec3c temp;
	vec3si curr_grad;
	multi_data=new vec3c[length];
	gradients=new vec3si[length];
	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
				curr=vec3i(x,y,z);
				diff=curr-center;
				distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
				if(distance<max_dst)
				{
					temp.x=unsigned char(255-255*distance/max_dst);
					temp.y=0;
					temp.z=0;
					curr_grad.x=short int(zero_val+double(zero_val*(diff.x/distance)));
					curr_grad.y=short int(zero_val+double(zero_val*(diff.y/distance)));
					curr_grad.z=short int(zero_val+double(zero_val*(diff.z/distance)));
					written++;
				}
				else
				{
					temp=vec3c(0,0,0);
					curr_grad=vec3si(zero_val,zero_val,zero_val);
				}
				write_multi_data(x,y,z,temp);
				write_grad_data(x,y,z,curr_grad);
				}
	vec3i mini_size(size.x/nr_balls,size.y/nr_balls,size.z/nr_balls);
	max_dst/=nr_balls;
	for(k=0;k<nr_balls;k++)
		for(j=0;j<nr_balls;j++)
			for(i=0;i<nr_balls;i++)
			{
			center.x=i*mini_size.x+mini_size.x/2;
			center.y=j*mini_size.y+mini_size.y/2;
			center.z=k*mini_size.z+mini_size.z/2;
			for(z=k*mini_size.z;z<(k+1)*mini_size.z;z++)
				for(y=j*mini_size.y;y<(j+1)*mini_size.y;y++)
					for(x=i*mini_size.x;x<(i+1)*mini_size.x;x++)
					{
						curr=vec3i(x,y,z);
						diff=curr-center;
						distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
						if(distance<max_dst)//&&distance>max_dst*0.6)
						{
							multi_data[x+y*size.x+z*(size.x*size.y)].y=unsigned char(255-255*distance/max_dst);
						}
					}

			}
	cout<<"writing data to disc"<<endl;
	output.write((char*)&multi_data[0],sizeof(vec3c)*length);
	grad_out.write((char*)&gradients[0],sizeof(vec3si)*length);
	cout<<written<<" voxels written in. Max_val "<<max_val<<endl;

	}

if(mode=='s')
	{
	cout<<"writing short ints with gradients"<<endl;
	//max_val=pow(256.0f,int(sizeof(grad_float)/4))/2-1;

	grad_float temp;
	all_data=new grad_float[length];

	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
				curr=vec3i(x,y,z);
				diff=curr-center;
				distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
				if(distance<max_dst&&distance>max_dst*0.6)
				{
					//temp.data=short int (abs(double(0.8*max_dst)-distance)*max_val*10);
					temp.data=short int(max_val);
					written++;
					//temp.gradients.x=diff.x;
					//temp.gradients.y=max_val;
					//temp.gradients.z=max_val;
					temp.gradients.x=short int(zero_val+double(max_val/2*diff.x)/distance);
					temp.gradients.y=short int(zero_val+double(max_val/2*diff.y)/distance);
					temp.gradients.z=short int(zero_val+double(max_val/2*diff.z)/distance);
				}
				else
				{
					temp.data=0;
					temp.gradients=vec3si(max_val/2,max_val/2,max_val/2);
				}
				write_data(x,y,z,temp);
				}
	output.write((char*)&all_data[0],sizeof(grad_float)*length);
	cout<<written<<" voxels written in. Max_val "<<max_val<<endl;
	}
}

