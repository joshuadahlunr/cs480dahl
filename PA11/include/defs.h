#ifndef CONFIG_H
#define CONFIG_H

enum CollisionGroups : int {		
    CG_ENVIRONMENT = 1 << 0,
    CG_UFO = 1 << 1,
    CG_COW = 1 << 2,
    CG_ALIEN = 1 << 3,
    
    CG_NONE = 0,
    CG_ALL = CG_ENVIRONMENT | CG_UFO | CG_COW | CG_ALIEN
};

#endif // CONFIG_H