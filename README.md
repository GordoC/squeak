# team-22

### Creative Elements

- [20] external integration - EnTT ecs package https://github.com/skypjack/entt, migrating our code from basic ecs to EnTT ecs
- [23] audio feedback - additional audio bytes played with new shooting action and pickup of items
- [24] basic integrated assets - hand drawn textures using procreate for different blocks and entities of the game


[20]
We apply interpolation for our screen darkening effect.

The code is in rendering.cpp in the code under the comment:

```// M1 interpolation implementation```

We use the following quatradic interpolation to apply an ease-in effect on our screen darkening time:

```a + (b - a) * t^2```

```a``` is our start value ```0``` and ```b``` is our end value ```1```. Our variable ```t``` is the time that has passed since the player has died. 

