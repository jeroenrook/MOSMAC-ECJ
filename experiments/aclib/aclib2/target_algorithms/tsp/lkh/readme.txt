Author: Yasha Pushak
Created: 2019-06-24
Last updated: 2019-06-24

I've added an extra categorical parameter to select whether or not the special WALK kick strategy is used. Normally this is selected with a value of 0 for KICK_TYPE and KICKS, but now this conversion is automatically handled within the wrapper.

KICK_TYPE therefore didn't have a default value, so I chose the middle of the specified range. 

I've done the same with SAME_MOVE_TYPE.

Based on what we observed in our landscape work, it seems like the same modification should have been made for BACKBONE_TRIALS (in fact, I believe that a value greater than 5 will give better performance than 0, but [0,5] is the range I was given to work with orignally).

[PURE] is an optional suffix for DELAUNAY for CANDIDATE_SET_TYPE. It doesn't seem to work properly, so I'm doing the same thing Zongxu once did and not using it (perhaps I just don't know how to specify it correctly, though).
