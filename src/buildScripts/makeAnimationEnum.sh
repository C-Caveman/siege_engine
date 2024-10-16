#!/bin/bash
# Generate animation X-Macro:
animationEnumFile='src/graphics/animations.h'
printf '#ifndef ANIMATIONS
#define ANIMATIONS 

#define ANIMATION_LIST(f) \\\n' > $animationEnumFile
ls -p assets/graphics/animations | 
    grep '/$' |
    sed 's/\/$//; s/^/    f(/; s/$/) \\/'  >> $animationEnumFile
printf '
enum animation_ids {
    ANIMATION_LIST(TO_ENUM)
    NUM_ANIM
};

#endif\n' >> $animationEnumFile
