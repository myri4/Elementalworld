#pragma once
#include <glm/glm.hpp>

namespace wc {
	class Entity {
	public:
		glm::vec3 Position = glm::vec3(0.0f);
		//glm::vec3 Size = glm::vec3(1.0f);
		//bool onGround = false;
		//float speed;
		virtual ~Entity() = default;
		virtual void Update() {}

		/*
		void collision(float Dx,float Dy,float Dz)
		{
		  for (int X=(x-w)/size;X<(x+w)/size;X++)
		  for (int Y=(y-h)/size;Y<(y+h)/size;Y++)
		  for (int Z=(z-d)/size;Z<(z+d)/size;Z++)
		    if (check(X,Y,Z))  {
		    if (Dx>0)  x = X*size-w; 
		    if (Dx<0)  x = X*size+size+w; 
			if (Dy>0)  y = Y*size-h; 
		    if (Dy<0) {y = Y*size+size+h; onGround=true; dy=0;} 
			if (Dz>0)  z = Z*size-d; 
		    if (Dz<0)  z = Z*size+size+d; 
		                       }
		}
		*/
	};
}