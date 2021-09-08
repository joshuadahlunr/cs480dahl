#ifndef MOON_H
#define MOON_H

#include "object.h"

class Moon: public Object {
public:
	void Update(unsigned int dt) override;

private:
	float angle;
};

#endif
