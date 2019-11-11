#define 	pi   3.14159265358

uniform sampler2D OutCoords,mTex;
uniform sampler3D mVolTex,mGradTex,mMiniTex;
uniform sampler1D TF;

uniform float threshold,div_coef,step,shininess,ratio;
uniform bool light,projection;	
uniform vec2 window_size;
uniform vec3 eye;

varying mat3 normal_matrix;
varying mat4 model_matrix;
varying vec3 normal;

vec4 CastRayTroughVolume(in vec4 RayIn,in vec4 RayOut)
{
vec4 color,diff,tmpCol,specular,diffuse,mini_col,texCol,par1,par2,voxCol,freckles,TFcol;
vec3 curr_coord,grad,halfvector,light_vec,vertex_pos,eye_pos,frag_pos,bubble_grad;
vec3 exact_cube,floor_cube,cube_coord;
vec2 exact_square,floor_square;
float dst,unit=1.0/float(step),opac_left=1.0,coef,steps=0.0,edge,parameter,angle,alfa,dot_bubble;
vec2 angles,tex_coords;
vec3 curr_pos;
int i;
bool enter,check_mini;
dst=abs(distance(RayIn,RayOut));
steps=dst/unit;
diff=(RayOut-RayIn)/steps;
check_mini=true;
color=vec4(0.0,0.0,0.0,0.0);
edge=threshold;
//edge=0.2;
//enter=false;
for(i=0;i<int(steps);i=i+1)
	{
	curr_coord=RayIn.rgb+diff.rgb*float(i);
	voxCol=texture3D(mVolTex,curr_coord);
	//TFcol=texture1D(TF,voxCol.r);
	TFcol=vec4(voxCol.r,voxCol.r,voxCol.r,voxCol.r);
	TFcol*=div_coef;	
	if(voxCol.r>edge)
	//if(TFcol.r>edge)
		{
		curr_pos=(curr_coord-0.5)*2.0;
		angles.x=acos(curr_pos.z/sqrt(pow(curr_pos.x,2.0)+pow(curr_pos.y,2.0)+pow(curr_pos.z,2.0)));
		angles.y=atan(curr_pos.y/curr_pos.x);
		tex_coords.x=angles.x/pi;
		tex_coords.y=(angles.y+pi/2.0)/pi;	//add pi cause i want result to be [0,1] for texture lookup
		exact_square=tex_coords*ratio;
		floor_square=floor(exact_square);
		tex_coords=exact_square-floor_square;
		texCol = texture2D(mTex,tex_coords);
		texCol=2.0*(texCol-0.5);

		cube_coord=curr_coord*ratio;
		//exact_cube=curr_coord*ratio;
		//floor_cube=floor(exact_cube);
		//cube_coord=exact_cube-floor_cube;
		freckles=(texture3D(mMiniTex,cube_coord));				
		bubble_grad=2.0*(freckles.rgb-0.5);
		parameter=voxCol.b;
		
		grad=texture3D(mGradTex,curr_coord).rgb;
		grad=normalize(2.0*(grad.rgb-0.5));
		bubble_grad=normalize(bubble_grad);
		dot_bubble=dot(grad,bubble_grad);
		angle = acos(dot_bubble);
		coef = freckles.a*sin(angle);
		//if(abs(coef)<parameter&&freckles.a>0.0&&abs(angle)<pi/2.0)grad=grad+3.0*bubble_grad;
		//coef=abs(bubble_grad.r)+abs(bubble_grad.g)+abs(bubble_grad.b);
		//if(coef>0.5)grad=grad+bubble_grad;
		//coef=(exp(freckles.a)-1.0)/(exp(1.0)-1.0);
		//if(freckles.a>0.0)grad=grad*coef+(1.0-coef)*dot_bubble*bubble_grad;
		if(abs(coef)<parameter&&freckles.a>0.0&&dot_bubble>0.0)grad=grad+sign(dot_bubble)*bubble_grad;
		
		//grad+=texCol.rgb;
		//grad=normalize(grad);
		grad=normalize(normal_matrix*grad);
		vertex_pos=vec3(gl_ModelViewMatrix*vec4(curr_coord,1.0));
		light_vec=normalize(gl_LightSource[0].position.xyz-vertex_pos);
		eye_pos=normalize(-vertex_pos);
								
		coef=max(dot(grad,light_vec),0.0);
		diffuse=coef*gl_LightSource[0].diffuse;
						
		//halfvector=normalize(light_vec+eye_pos);
		//coef=max(dot(grad,halfvector),0.0);
		//if(voxCol.g!=0.0)specular=pow(coef,shininess/sqrt(voxCol.g))*gl_LightSource[0].specular;
		//else specular=gl_LightSource[0].specular*0.0;
		//specular=pow(coef,shininess)*gl_LightSource[0].specular;
		
			
		//if(light)color+=opac_left*tmpCol.a*tmpCol*(diffuse+specular*texCol*voxCol.g);
		//if(light)color+=opac_left*TFcol*(diffuse);
		color+=opac_left*TFcol*TFcol.a*diffuse;//+texCol.r*voxCol.b*div_coef*texCol);
		//color+=opac_left*TFcol*(diffuse);
 		opac_left*=(1.0-TFcol.a);
		if(opac_left<0.01)break;		//wonderful speedup
		}	
	}
return color ;
}

void main()
	{
		vec2 inout_coords=gl_FragCoord.xy/window_size;
		vec4 color;
		vec4 RayOutCoord = texture2D(OutCoords,inout_coords);
		vec4 RayInCoord = gl_Color;
		color = CastRayTroughVolume(RayInCoord,RayOutCoord);
		gl_FragColor = color;
	}