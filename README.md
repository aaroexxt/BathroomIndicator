# BathroomIndicator
 How do you know if someone's in the bathroom? A custom PCB, of course...

## Design Intent
I built this mostly to practice my PCB design and layout skills, but it's actually quite cool in person. Basically, it's designed to be a comprehensive system for people who have to share space to have a way to tell others that they're busy. I've installed a set of these on a bathroom I share with my sister (who thought that was a good idea?) so that we can both know when somebody else is using the bathroom.

## Pictures

Version 1
![Version 1](/Pictures/V1.JPG)


Version 2
![Version 2](/Pictures/V2.JPG)

## Renderings

Top - Version 2
![Version 2 Top](/Pictures/V2Top.png)

Bottom - Version 2
![Version 2 Bottom](/Pictures/V2Bottom.png)

### Design Notes

The major difference between versions 1 and 2 is that Version 1 had a custom radio based on the NRF24L01+ radio by Nordic Semiconductor, while this was replaced with a premade module in Version 2 because of my lack of experience with antenna design (1 foot range on my custom one vs _30_ feet on the premade module). Other than that, it worked well. I also had some issues with the 3.3v regulator schematic in V1 (incorrect schematic for EN pin) but this was solved in version 2.

### By Aaron Becker