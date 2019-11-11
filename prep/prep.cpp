#include <iostream>
#include <fstream>
#include <sstream>	//for istringstream
#include <algorithm>
#include "vec3.h"

using namespace std;

struct bmp_header	//54 bytes
{
   // unsigned short bfType;           /* Magic number for file 2*/
    unsigned int   bfSize;           /* Size of file 4*/
    unsigned short bfReserved1;      /* Reserved 2*/
    unsigned short bfReserved2;      /* ... 2*/
    unsigned int   bfOffBits;        /* Offset to bitmap data 4*/
    unsigned int   biSize;           /* Size of info header 4*/
    int            biWidth;          /* Width of image 4*/
    int            biHeight;         /* Height of image 4*/
    unsigned short biPlanes;         /* Number of color planes 2*/
    unsigned short biBitCount;       /* Number of bits per pixel 2*/
    unsigned int   biCompression;    /* Type of compression to use 4*/
    unsigned int   biSizeImage;      /* Size of image data 4*/
    int            biXPelsPerMeter;  /* X pixels per meter 4*/
    int            biYPelsPerMeter;  /* Y pixels per meter 4*/
    unsigned int   biClrUsed;        /* Number of colors used 4*/
    unsigned int   biClrImportant;   /* Number of important colors 4*/
};


typedef vec3c in_type;
typedef vec3si out_type;
//typedef unsigned short int down_intype;
typedef unsigned char down_intype;
typedef unsigned char down_outtype;
typedef unsigned char mini_type;
typedef unsigned char real_intype;
typedef vec3c real_outtype;

unsigned char *in_data;
out_type *all_data;
in_type *raw_data;
mini_type *mini_data;
real_intype *r_indata;
real_outtype *r_outdata;
down_intype *down_in;
down_outtype *down_out;
vec3i size,mini_size,nr_mini_voxels;

//unsigned char read_data(int x,int y,int z){return raw_data[x+y*size.x+z*(size.x*size.y)];}
//unsigned char read_data(int x,int y,int z){return raw_data[x+y*size.x+z*(size.x*size.y)];}
unsigned char read_data_x(int x,int y,int z)
{return raw_data[x+y*size.x+z*(size.x*size.y)].x;}

real_intype read_real_data(int x,int y,int z)
{return r_indata[x+y*size.x+z*(size.x*size.y)];}

unsigned char read_data(int x,int y,int z)
{return in_data[x+y*size.x+z*(size.x*size.y)];}

void write_data(int x,int y,int z,out_type tmp)
{all_data[x+y*size.x+z*(size.x*size.y)]=tmp;}

void write_mini_data(int x,int y,int z,mini_type tmp)
{mini_data[x+y*mini_size.x+z*(mini_size.x*mini_size.y)]=tmp;}

bmp_header set_up_header(vec3i size);

int main(int argc,char* argv[])
{
istringstream line;
if (argc<3){cout<<"program usage: prep[1] filename[2] mode[3] scale[4](opt)"<<endl
<<"mode listing:"<<endl
<<"g - calculate gradients for given dataset for first component of the dataset"<<endl
<<"a - (not implemented yet) smooth the volume with given filter kernel"<<endl
<<"m - produce miniature dataset with avaraged values 'scale' times smaller on each axis"<<endl
<<"r - calculate gradients in one file and adds second and third parameter to existing dataset"<<endl
<<"		reguires additional [4] parameter specifying number of small spheres as a 3rd parameter"<<endl
<<"p - marges 2d images into one 3d texture.Pictures MUST be numbered from 1.bmp to z.bmp"<<endl
<<"z - save one slice in file , slice number specified by 4th parameter"<<endl
<<"d - downsample from 16 bits to 8 bits, later need more preprocessing i.e. gradient creation"<<endl
<<"s - stretch 3d volume along z direction to match size.x direction (make it cubic)"<<endl<<endl
<<"example : prep volume.raw r 3"<<endl
<<"REMEMBER - you need to create file filename.size with the x,y,z sizes of the volume"<<endl;exit(1);}
int z,x,y,i,j,k;
int length,coef;//coefficent to change char to short int
int zero_val;//the value of zero in shader 32768/2
int scale;
float distance;
string in_name,size_name,grad_name,modi_name;
char mode;
//float max=255;

mode=argv[2][0];
//if(mode=='m'||mode=='r')
scale=atoi(argv[3]);
in_name=argv[1];

size_name=in_name+".size";
ifstream sizes(size_name.c_str(),ios::in);	//and open file name.size with dimensions
sizes>>size.x>>size.y>>size.z;	//read sizes in
cout<<size.x<<" "<<size.y<<" "<<size.z<<" "<<size_name<<endl;
length=size.x*size.y*size.z;

int nr_balls=scale;
int max_dst=min(size.x/2,size.y/2);
max_dst=min(max_dst,size.z/2);

vec3i diff,curr,center;
center.x=size.x/2-1;
center.y=size.y/2-1;
center.z=size.z/2-1;


if(mode=='z')
{
	int pic_length=size.x*size.y;
	unsigned short type=19778;
	string out_name;
	stringstream number;
	bmp_header header;
	unsigned char *raw_in,*pic;
	ofstream output;

	raw_in=new unsigned char[length];
	pic=new unsigned char[3*pic_length];

	header=set_up_header(size);

	ifstream input(in_name.c_str(), ios::in | ios::binary);
	input.read ((char*)raw_in,length);
	//number<<scale;
	system("mkdir pictures");
	//out_name="pictures/"+number.str()+in_name+".bmp";
	//output(out_name.c_str(), ios::out | ios::binary);

	for(j=0;j<size.z;j++)
	{
		stringstream number;
		number.flush();
		number<<j+1<<"_";
		out_name="pictures/"+number.str()+in_name+".bmp";
		ofstream output(out_name.c_str(), ios::out | ios::binary);

		for(i=0;i<pic_length;i++)
		{
		pic[3*i]=raw_in[pic_length*j+i];
		pic[3*i+1]=raw_in[pic_length*j+i];
		pic[3*i+2]=raw_in[pic_length*j+i];
		}

		//cout<<"writing to disc"<<endl;
		//!!!!!!!!! sooo stupid , structure or class can be only power of 2
		//it can't have size 54 , only 52 or 56 !!!! 
		output.write((char*)&type,sizeof(unsigned short));
		output.write((char*)&header,sizeof(bmp_header));
		//cout<<sizeof(bmp_header)<<endl;
		output.write((char*)&pic[0],3*pic_length);
	}
	output.close();
}

if(mode=='d')
{
	string out_name;

	down_in=new down_intype[length*2];
	down_out=new down_outtype[length];

	ifstream input(in_name.c_str(), ios::in | ios::binary);
	input.read ((char*)down_in,length*sizeof(down_intype));
	out_name="8bit_"+in_name;
	ofstream output(out_name.c_str(), ios::out | ios::binary);

	for(i=0;i<length;i++)
	{
	//down_out[i]=down_outtype(float(down_in[i])/float(256.0));
	down_out[i]=down_in[2*i];
	}

	cout<<"writing to disc"<<endl;
	output.write((char*)&down_out[0],sizeof(unsigned char)*length);
	output.close();
}

if(mode=='s')
{
	int out_length,pic_length;
	float ratio,e,f,c,w1,w2;//ratio,exact,floot,ceil,weight1,weight2
	unsigned char *out_data,*tmp_data;
	string out_name;

	out_length=size.x*size.y*size.x;
	pic_length=size.x*size.y;
	out_data=new unsigned char[out_length];
	tmp_data=new unsigned char[length];
	ifstream input(in_name.c_str(), ios::in | ios::binary);
	input.read ((char*)tmp_data,length);
	out_name="new_"+in_name;
	ofstream output(out_name.c_str(), ios::out | ios::binary);

	ratio=(float)size.z/(float)size.x;
	for(i=0;i<size.x;i++)
	{
	e=i*ratio;
	f=floor(e);
	c=ceil(e);
	w1=1-(e-f);
	w2=1-w1;
	if(c==size.z)c=f;
	for(j=0;j<pic_length;j++)
		{
		out_data[i*pic_length+j]=unsigned char(w1*(float)tmp_data[(int)f*pic_length+j]+w2*(float)tmp_data[(int)c*pic_length+j]);
		}
	}

	cout<<"writing to disc"<<endl;
	output.write((char*)&out_data[0],sizeof(unsigned char)*pic_length*size.x);
	output.close();
}

if(mode=='p')
{
	int pic_nr=1,pic_length;
	float ratio,e,f,c,w1,w2;//ratio,exact,floot,ceil,weight1,weight2
	unsigned char *out_data,*tmp_data,*buffer;
	string pic_name;

	pic_length=size.x*size.y;
	out_data=new unsigned char[pic_length*size.x];
	tmp_data=new unsigned char[length];
	buffer=new unsigned char[pic_length*3];
	ofstream output(in_name.c_str(), ios::out | ios::binary);

	for(i=1;i<=size.z;i++)
	{
	stringstream ss;
	ss << i << ".bmp";
	pic_name=ss.str();
	cout<<pic_name<<endl;
	ifstream input(pic_name.c_str(), ios::in | ios::binary);
	//input.seekg(0,54);
	input.seekg(54, ios::beg);
	input.read ((char*)buffer,size.x*size.y*3);
	for(j=0;j<pic_length;j++)
		tmp_data[(i-1)*pic_length+j]=buffer[3*j];
	input.close();
	}
	ratio=(float)size.z/(float)size.x;
	for(i=0;i<size.x;i++)
	{
	e=i*ratio;
	f=floor(e);
	c=ceil(e);
	w1=1-(e-f);
	w2=1-w1;
	if(c==size.z)c=f;
	for(j=0;j<pic_length;j++)
		{
		out_data[i*pic_length+j]=unsigned char(w1*(float)tmp_data[(int)f*pic_length+j]+w2*(float)tmp_data[(int)c*pic_length+j]);
		}
	}

	cout<<"writing to disc"<<endl;
	output.write((char*)&out_data[0],sizeof(unsigned char)*pic_length*size.x);
	output.close();
}
//calculate 
if(mode=='a')
{
	string out_name;
	unsigned char *out_data;
	float tmp;
	in_data=new unsigned char[length];
	ifstream luminance(in_name.c_str(),ios::in|ios::binary);	//stream for input file
	luminance.read((char*)(&in_data[0]),length*sizeof(unsigned char));

	out_name="smooth_"+in_name;
	ofstream output(out_name.c_str(), ios::out | ios::binary);	//stream for output file  name.size.my
	out_data=new unsigned char[length];
	for(z=0;z<size.z;z++)
	{
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
				if(x==0||x==size.x-1||y==0||y==size.y-1||z==0||z==size.z-1)
					out_data[x+y*size.x+z*(size.x*size.y)]=read_data(x,y,z);
				else
					{
					tmp=float(
					read_data(x,y,z)+
					read_data(x-1,y,z)+
					read_data(x+1,y,z)+
					read_data(x,y-1,z)+
					read_data(x,y+1,z)+
					read_data(x,y,z-1)+
					read_data(x,y,z+1));
					tmp=tmp/7;
					out_data[x+y*size.x+z*(size.x*size.y)]=unsigned char(tmp);
					}
				}
	}
		cout<<"writing data to file"<<endl;
		output.write((char*)&out_data[0],sizeof(unsigned char)*length);
}

//gradients
if(mode=='g')
{
	ifstream luminance(in_name.c_str(),ios::in|ios::binary);	//stream for input file

	grad_name=in_name+".grad";
	ofstream output(grad_name.c_str(), ios::out | ios::binary);	//stream for output file  name.size.my

	vec3 tmp_max=vec3(0,0,0),tmp_min=vec3(0,0,0);
	out_type tmp;

	zero_val=32768/2;
	coef=256/2;

	raw_data =new in_type[length];
	all_data=new out_type[length];

	luminance.read((char*)(&raw_data[0]),unsigned(length*sizeof(in_type)));
	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
						if(x==0||x==size.x-1)					
							tmp.x=zero_val;
						else tmp.x=zero_val+coef*(read_data_x(x+1,y,z)-read_data_x(x-1,y,z));
						if(y==0||y==size.y-1)					
							tmp.y=zero_val;
						else tmp.y=zero_val+coef*(read_data_x(x,y+1,z)-read_data_x(x,y-1,z));
						if(z==0||z==size.z-1)					
							tmp.z=zero_val;
						else tmp.z=zero_val+coef*(read_data_x(x,y,z+1)-read_data_x(x,y,z-1));
						if(tmp.x>tmp_max.x)tmp_max.x=tmp.x;
						if(tmp.y>tmp_max.y)tmp_max.y=tmp.y;
						if(tmp.z>tmp_max.z)tmp_max.z=tmp.z;
						if(tmp.x<tmp_min.x)tmp_min.x=tmp.x;
						if(tmp.y<tmp_min.y)tmp_min.y=tmp.y;
						if(tmp.z<tmp_min.z)tmp_min.z=tmp.z;
						write_data(x,y,z,tmp);
					}
		cout<<"writing data to file"<<endl;
		zero_val=2*zero_val-1;	//set the maximum value (for data to texture transformation-scaling)
		tmp=vec3si(zero_val,zero_val,zero_val);
		write_data(0,0,0,tmp);
		output.write((char*)&all_data[0],sizeof(out_type)*length);
		cout<<"x:"<<tmp_max.x<<" "<<"y:"<<tmp_max.y<<" "<<"z:"<<tmp_max.z<<std::endl;
		cout<<"x:"<<tmp_min.x<<" "<<"y:"<<tmp_min.y<<" "<<"z:"<<tmp_min.z<<"float"<<sizeof(out_type)<<std::endl;
}

if(mode=='r')
{
	ifstream luminance(in_name.c_str(),ios::in|ios::binary);	//stream for input file
	grad_name=in_name+".grad";
	ofstream output(grad_name.c_str(), ios::out | ios::binary);	//stream for output file  name.size.my
	modi_name=in_name+".modi";
	ofstream modified(modi_name.c_str(), ios::out | ios::binary);	//stream for output file  name.size.my

	vec3 tmp_max=vec3(0,0,0),tmp_min=vec3(0,0,0);
	out_type tmp;

	zero_val=32768/2;
	coef=256/2;

	r_indata = new real_intype[length];
	r_outdata = new real_outtype[length];
	all_data = new out_type[length];

	cout<<"preprocessing dataset - creating gradients"<<endl;

	luminance.read((char*)(&r_indata[0]),unsigned(length*sizeof(real_intype)));
	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
						if(x==0||x==size.x-1)					
							tmp.x=zero_val;
						else tmp.x=zero_val+coef*(read_real_data(x+1,y,z)-read_real_data(x-1,y,z));
						if(y==0||y==size.y-1)					
							tmp.y=zero_val;
						else tmp.y=zero_val+coef*(read_real_data(x,y+1,z)-read_real_data(x,y-1,z));
						if(z==0||z==size.z-1)					
							tmp.z=zero_val;
						else tmp.z=zero_val+coef*(read_real_data(x,y,z+1)-read_real_data(x,y,z-1));
						if(tmp.x>tmp_max.x)tmp_max.x=tmp.x;
						if(tmp.y>tmp_max.y)tmp_max.y=tmp.y;
						if(tmp.z>tmp_max.z)tmp_max.z=tmp.z;
						if(tmp.x<tmp_min.x)tmp_min.x=tmp.x;
						if(tmp.y<tmp_min.y)tmp_min.y=tmp.y;
						if(tmp.z<tmp_min.z)tmp_min.z=tmp.z;
						write_data(x,y,z,tmp);
					}
		cout<<"writing gradients to file...";
		zero_val=2*zero_val-1;	//set the maximum value (for data to texture transformation-scaling)
		tmp=vec3si(zero_val,zero_val,zero_val);
		write_data(0,0,0,tmp);
		output.write((char*)&all_data[0],sizeof(out_type)*length);
		cout<<"done"<<endl;

	cout<<"rewriting original data and adding new 2Dims artificial data (spheres - green and blue component)"<<endl;
	for(z=0;z<size.z;z++)
		for(y=0;y<size.y;y++)
			for(x=0;x<size.x;x++)
				{
				r_outdata[x+y*size.x+z*(size.x*size.y)].x=read_real_data(x,y,z);
				r_outdata[x+y*size.x+z*(size.x*size.y)].z=0;
				curr=vec3i(x,y,z);
				diff=curr-center;
				distance=sqrt(pow(float(diff.x),2)+pow(float(diff.y),2)+pow(float(diff.z),2));
				if(distance<max_dst)
					r_outdata[x+y*size.x+z*(size.x*size.y)].y=unsigned char(255-255*distance/max_dst);
				else
					r_outdata[x+y*size.x+z*(size.x*size.y)].y=0;
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
						if(distance<max_dst)
						{
							r_outdata[x+y*size.x+z*(size.x*size.y)].z=unsigned char(255-255*distance/max_dst);
						}
					}

			}
	cout<<"writing data to disc..";
	modified.write((char*)&r_outdata[0],sizeof(real_outtype)*length);
	cout<<"done"<<endl;
}


if(mode=='m')
{
	ifstream luminance(in_name.c_str(),ios::in|ios::binary);	//stream for input file
	grad_name=in_name+".mini";
	ofstream output(grad_name.c_str(), ios::out | ios::binary);	//stream for output file  name.size.my


	vec3 tmp_max=vec3(0,0,0),tmp_min=vec3(0,0,0);
	mini_type tmp;
	int mini_length,max,min;
	unsigned long sum;
	
	mini_size=size/scale;
	mini_length=mini_size.x*mini_size.y*mini_size.z;
	raw_data =new in_type[length];
	mini_data=new mini_type[mini_length];

	luminance.read((char*)(&raw_data[0]),unsigned(length*sizeof(in_type)));
	for(k=0;k<mini_size.z;k++)
		for(j=0;j<mini_size.y;j++)
			for(i=0;i<mini_size.x;i++)
			{
			sum=0;
			max=0;
			min=9999999;
			for(z=k*scale;z<(k+1)*scale;z++)
				for(y=j*scale;y<(j+1)*scale;y++)
					for(x=i;x<(i+1)*scale;x++)
					{
						sum+=read_data_x(x,y,z);
						//if(read_data_x(x,y,z)>max)max=read_data_x(x,y,z);
						//if(read_data_x(x,y,z)<min)min=read_data_x(x,y,z);
					}
			tmp=(mini_type)(sum/pow((float)scale,3));	//divide sum by number of voxels contained in voxel of mini volume
			//tmp=max;
			//tmp=min;
			//printf("%d ",tmp);
			write_mini_data(i,j,k,tmp);
			}

		cout<<"writing data to file...";
		output.write((char*)&mini_data[0],sizeof(mini_type)*mini_length);
		cout<<"done"<<endl<<"x:"<<mini_size.x<<" y:"<<mini_size.y<<" z:"<<mini_size.z<<endl;
}

}

bmp_header set_up_header(vec3i size)
{
bmp_header tmp;

//tmp.bfType = 19778;
tmp.bfSize = 54+3*size.x*size.y;
tmp.bfReserved1 = 0;
tmp.bfReserved2 = 0;
tmp.bfOffBits = 54;

tmp.biSize = 40;
tmp.biWidth = size.x;
tmp.biHeight = size.y;
tmp.biPlanes = 1;
tmp.biBitCount = 24;
tmp.biCompression = 0;
tmp.biSizeImage = 0;
tmp.biXPelsPerMeter = 0;
tmp.biYPelsPerMeter = 0;
tmp.biClrUsed = 0;
tmp.biClrImportant = 0;

//tmp.rgbBlue=0;
//tmp.rgbGreen=0;
//tmp.rgbRed=0;
//tmp.rgbReserved=0;

return tmp;
}