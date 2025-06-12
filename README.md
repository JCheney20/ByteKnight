# JUST-IN-CHECK

![J-i-C Logo](./src/img/cover_logo.png) 

Just-in-Check is a Chess Engine written in C following the video tutorial created by [bluefeversoft](https://github.com/bluefeversoft/vice). It was an experience to learn how to use various aspects of C, better my understanding of Artificial Intelligence as I create heuristics to calculate best moves as well as getting to apply what has been learnt during my third year of university. 

## Additions 
I have made a few additions to this project over and above the tutorial include:
- Improving the PST (Piece-Square Tables)
- Automated Tuning (Making use of learning modes to allow for on going changes to the PST to allow for linear and automated improvement of the position evaluations)

## TO-DO 
- Create own UI using [Clay](https://github.com/nicbarker/clay) and possibly [raylib](https://www.raylib.com/) to create a almost 100% C based program.
- Implement autotuning

    ### fix bugs: 
    - Unable to use books when doing multithreading, causes illegal moves
    - The PickMove function needs a BestScore of -(very low) instead of 0
    - Time management for x moves in x minutes causes losses
    - ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)

