varying mat3 normal_matrix;
varying mat4 model_matrix;
varying vec3 normal;


void main()
	{	
		gl_FrontColor = gl_Color;	
		normal_matrix= gl_NormalMatrix;
		normal=gl_Normal;
		model_matrix=gl_ModelViewMatrix;
		//vertex=gl_ModelViewMatrix*gl_Vertex;
		gl_Position = ftransform();
	}