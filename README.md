zoom430
=======

silly little msp430 launchpad game

This uses the two buttons as left and right and you have to navigate a
dangerous chasm! Looks kinda like this and scrolls down:
```
|    |
 |   |
 |    |
|    |
|   |
 | V |
```

It is able to use the reset button by hooking the nmi interrupt. This means
you'll need to program it and then stop debugging it before you can
actually play it :)
