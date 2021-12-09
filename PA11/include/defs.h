#ifndef CONFIG_H
#define CONFIG_H

enum CollisionGroups : int {		
    Enviornment = 1 << 0,
    UFO = 1 << 1,
    Cow = 1 << 2,
    Person = 1 << 3,
    
    None = 0,
    All = Enviornment | UFO | Cow | Person
};

#endif // CONFIG_H