#include <proto/protographics.h>
#include <proto/color_utils.h>
#include <proto/math/math_common.h>
#include <string>

namespace {
		protowizard::ProtoGraphicsPtr proto;
}

glm::vec2 pointOnCicle( float u )
{
	return glm::vec2( cos(u), sin(u) );
}

void circle_of_things()
{
	//proto->setLightBlend();
	proto->setLightBlend();
	proto->setAlpha(1.0f);

	float time = (float)proto->klock();

	int num_things = 12;
	for ( int i=0; i<num_things; i++ ){
		float param = i / float(num_things);
		float ang =  param * (TWO_PI);
		glm::vec2 point = 10.f * pointOnCicle( ang ); 
		float normalized = ang/(TWO_PI);
		float hang = normalized * 360.f;

		proto->setColor( protowizard::hsv2rgb( hang, 1,1 ) ); 
		proto->setTexture( proto->getResourceDir() + std::string("/textures/alpha_particle.png") );
		float ca = cos(4.f*time + ang);
		float sa = sin(4.f*time + ang);
		proto->drawPlane( glm::vec3(point.x, 0.f, point.y), glm::vec3(ca, 0.f, sa), 2.f );
	}
}

int main(int argc, const char* argv[])
{
	proto = protowizard::ProtoGraphics::create();

	if (!proto->init(640,480) ) {
		throw char("proto failed to init. probably shaders not found or GL drivers");
		return 1;
	}


	size_t numframes = 0;

	glm::vec3 cam_pos(0.f, 2.f, -15.f);
	glm::vec3 cam_target = glm::vec3(0.f, 0.f, 1.f);


	proto->setTitle( "your friendly example project" );
	proto->setFrameRate( 60 );
	do
	{
		proto->cls(0.5f, 0.5f, 0.5f);
		
		circle_of_things();
		
		proto->setBlend(false);
		proto->setAlpha(1.f);

		proto->disableTexture();
		for(int i=0; i<10; i++)
		{
			float normalized = i / 10.f;
			proto->setColor( protowizard::hsv2rgb( 360.f * normalized, 1,1 ) ); 
			//proto->setTexture("assets/textures/alpha_particle.png");
			glm::vec3 pos = glm::vec3(0.f, 0.f, normalized * 20.f);
			//proto->drawPlane( pos, glm::vec3(0.f, 0.f, 1.f), 2.f );
			proto->setScale( 0.5f + normalized * 2.f );
			proto->drawCube( pos );
		}

		
		proto->setBlend( true );
		proto->setAlpha( 0.5f );
		for(int i=0; i<10; i++)
		{
			float normalized = i / 10.f;
			proto->setColor( protowizard::hsv2rgb( 360.f * normalized, 1,1 ) ); 
			//proto->setTexture("assets/textures/alpha_particle.png");
			glm::vec3 pos = glm::vec3(-5.f, 0.f, normalized * 20.f);
			//proto->drawPlane( pos, glm::vec3(0.f, 0.f, 1.f), 2.f );
			proto->setScale( 0.5f + normalized * 2.f );
			proto->drawCube( pos );
		}

		proto->setLightBlend();
		proto->setAlpha( 0.5f );
		for(int i=0; i<10; i++)
		{
			float normalized = i / 10.f;
			proto->setColor( protowizard::hsv2rgb( 360.f * normalized, 1,1 ) ); 
			//proto->setTexture("assets/textures/alpha_particle.png");
			glm::vec3 pos = glm::vec3(+5.f, 0.f, normalized * 20.f);
			//proto->drawPlane( pos, glm::vec3(0.f, 0.f, 1.f), 2.f );
			proto->setScale( 0.5f + normalized * 2.f );
			proto->drawCube( pos );
		}
		proto->setScale(1.f);

		float revolutions_per_second = TWO_PI / 10.f;
		float spin = revolutions_per_second * (float)proto->klock();

		cam_pos = 10.f * glm::vec3( sin(spin), 0.f, cos(spin) );
		proto->getCamera()->lookAt( cam_pos, cam_target, glm::vec3(0.f, 1.f, 0.f) );
		

		proto->frame();
		numframes++;
	} while( proto->isWindowOpen() );

	proto.reset();

}