#ifndef MOON_H
#define MOON_H

#include "object.h"

class Moon: public Object {
public:
	void Update(unsigned int dt) override;

	void setScale(float scale) { this->scale = scale; }
	float getScale() { return scale; }

private:
	float angle;
	float scale = .75;
};

#endif
