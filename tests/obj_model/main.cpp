#include <protographics.h>
#include "color_utils.h"



void draw_models( ProtoGraphics &proto )
{
	proto.setScale( 1.f );
	static float horiz = 0.f;
	horiz += proto.getMSPF() * 45.f;

	proto.disableTexture();
	
	proto.setTexture( "assets/textures/googley_hen.jpg");
	proto.drawMesh( glm::vec3(0.f, 0.5f, 0.f), horiz, 90.f, "assets/models/googley_chicken.obj");


	proto.setTexture( "assets/textures/cube.png");
	proto.drawMesh( glm::vec3(0.f, 0.f, 0.f), horiz, 0, "assets/models/cube.obj");
}


int main()
{
	ProtoGraphics proto;

	if (!proto.init(640,480) ) {
		throw char("proto failed to init. probably shaders not found or GL drivers");
		return 1;
	}


	proto.setFrameRate( 60 );
	

	
	
	char title_buf[256];
	do
	{
		proto.cls(0.f, 0.f, 0.f);

		float mspf = proto.getMSPF();
		sprintf_s( title_buf, 256, "%.2f mspf, %3.0f", 1000.f * mspf, 1.f/mspf);
		proto.setTitle( title_buf );

		float normalized_mx = proto.getMouseX() / (float)proto.getWindowWidth();
		float normalized_my = proto.getMouseY() / (float)proto.getWindowHeight();
		glm::vec2 normalized_mouse(normalized_mx, normalized_my);

		float zoom = normalized_my * 5.f;
		proto.setCamera( glm::vec3(0.f, zoom, -zoom), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.f, 0.f) );
		draw_models( proto );

		//float ang = normalized_mouse.x*6.28f;
		//glm::vec3 cam_pos( cos(ang), 0.f, sin(ang) );
		//cam_pos *= 25.f;
		//proto.setCamera( cam_pos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) );

		proto.frame();

	} while( proto.isWindowOpen() );


}