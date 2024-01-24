# Big Mouth Billy
 Arduino code to animate Big Mouth Billy Bass
 * billy_test.ino: Debug file to check sound input coming in and to see if motors were wired up correctly. This file doesn't try to map sounds to movement. The only purpose of this file was to see if I could detect sound from the pins and if I could control movement.

* billy.ino: An attempt to create a more "state" based feel to the bass. The idea is to check if there is sound above a certain threshold. If there is, then simulate mouth flapping for a short period of time, then check for sound again. If there is sound then flap mouth. This logic is similar for the tail/body but I toggle between different rest states (full/half/quarter) to simulate the effect of moving the body back and forth while still allowing for the mouth to move.
