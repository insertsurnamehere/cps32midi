# cps32midi
MADE IN COLLABORATION WITH VGMRIPS.NET https://vgmrips.net/forum/viewforum.php?f=15

Cps32midi is a sequence converter for the games that where first (or only) released on capcom's last (and shortlived) arcade system:
Capcom Play System 3 (cps3 for short).
It included a wide selection of 6 games (impressive), of which:

3 are the 3 versions of street figther 3 (new generation, 2nd giant attack, and 3rd strike);

2 are the 2 versions of the beloved jojo part 3 figthing game: 
         jba: herritage for the future (which everyboy and their grandma plays)
         and jojo's venure (which nobody plays, except for the grandmas that don't know there are 2 versions of this game, because it sucks, 
         and a sin to humanity >>>>>>>>>>>>>>:(    );

and 1 single, lonely, game, that game begin red heart/warzard, that nobody knows about, that gets relatively no love, 
an interesting figthing game with rpg elements, 
that was the first title ever released for the cps3, that died, 
because it was only released there (there where no ports for this game to my knowledge), 
and rests in the grave of it <<<<<<<:(

but enough chit chat, this converter needs works with the sample data, instrument data, and the sequence data, separated in 3 bin files, that can be ripped with mame's debugger commands:

https://docs.mamedev.org/debugger/memory.html#debugger-command-save

The placement of these are different for each game, and migth be dependent on the configuration of the simms 

(basicly, in the board itself, there are 7 slots where a memory expansion chip can be placed, for this boards there are 3 types of chips (these are the simms) that can be placed in these slots, and each game REQUIRES a specific arrangement of these types of chips, and maybe that can shifth the actual addresses of the sample, instrument, and sequence data)

One advice for using this program: DONT
unless you hav some expirience with the mame debugger, i don't know of a reliable way to find it, but there is a sound test, that cloud be useful, and these have a "consistent" byte size, but if you value your sanity: DONT.

Here is the structure of the samplebank, instrumentbank and the sequence data.

Sample bank:
its simpler than the other two, should be the easiest to find, 

its comptised of a series of 4 word-long values,
the first word is the sample start pointer, the second is the loop start pointer, the third is the end sample pointer, and the last is the center key.
EXAMPLE: 0000122A 00003CB2 00003CB2 00000050
the pointers are applied to the huge sample table, that can be found in the simm3.* files inside the rom of a cps3 game, though these need to be interleaved in couples (simm3.0 interleaved with simm3.1, simm3.2 with simm3.3, and so on), then combined.

Instrumentbank:
its similar to soundfonts in concept, but a lot less complicated in the file structure, without any of that complicated chunk stuff,

the first 16 word sized pointers point to the offsets of each instruments, then adding the pointers and the instrument offsets and you get the locaton of each instrument.

the actual data is similar to the cps2, its composed of 12 bytes, for each keysplit
EXAMPLE: 29 FF 10 50 00 56 00 3F 3F 7F 01 3F ...
29(1st byte) is the highest key for the keysplit, 56(6th byte) is the sample id 
(you need to count the samples starting with 0 to know the sample used), 3F 3F 7F 01 3F (last 5 bytes) are the attack/delay/hold/sustain/release envelopes, and these work similarly to how the cps2 handled adsr (i took inspiration from vgmtans's qsound converter (i meant "stole") 
also the cps3 updates the sound code at a different speed than the cps2), FF(2nd byte) migth be the volume... i don't really know the rest...
if a keysplit is then followed by FF FF, that makes it the last keysplit in the instrument: that instrument's data ends here
EXAMPLE: 30 FF 10 44 00 5C 00 3F 3F 7F 01 3F FF FF

sequence data:
first, a list of pointer offsets that indicate the location of a singular song, then there are 18 short-sized values, 
the first value i dont know what it is, but following it there are the channel offsets, add the song offset and the channel offset to get to the address of the actual song data, 
and its similar to midi, with a list of commands that have a similar functions, and each having a byte length, usually 2...
if the command is less than 0x80, then its a delay 
(the program waits the number of thicks indicated, like in a midi file with ppqn of 48, also, if followed by another byte that is less than 0x80, then combine them by first shifthing left, by 7 bits and bitwise OR with the next byte)
if the command is greater than 0x80, but smaller than C0, then its a note trigger, and this byte is the volume/velocity, next byte is the note (max 128), 
and after that its the note duraton, acts like the delay command, but it can only be a 16 bit value maximum
EXAMPLE: BF 3C 81 40
BF is the volume, 3C = note, and 81 40 is the duration, if the 3rd byte (81) was less than 80,  then it would become a single byte.

if the command is greater or equal to C0, then it is an effect, inside the code i made comments on what those effects are...
the tables (for vibrato value, adsr, etc) are the same as in the cps2, but i think these are processed differently, obviusly.


i neeed more comments...
it's a work in progress, i need to finish the vibrato code, also i don't like the mixing (either the volume command, or the sustain is at fault here).

i know my descriptions suck, 
deal  with it >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>:(

JOJO MIDIS















IMPORTANT NOTES

didn't write about this before, you need to make sure the simm3 files are formatted correcly.

first, delete any simm3 file without samples(open each one on audacity (file->import->audio), and see if these have noise instead)
then, delete any prefix (like the acronym for the game(like sfiii3-simm3.x) in a mame rom), and renaming them so the files have exactly the  name "simm3.x"


ex:("sfiii3-simm3.0", "sfiii3-simm3.1", "sfiii3-simm3.2", and "sfiii3-simm3.3", should be named "simm3.0", "simm3.1", "simm3.0", etc)


on instrument chunk (or any chunk), don't start a file ripping from a blank space, start from the first pointer (or first entry, in the sample chunk, or in the seqence chunk, rip from exacly 8 bytes before the actual firs song pointer  (es: 0x50 00 00 00 00 00 00 00))

don't rip samplechunk with blank samples, but exacly until the last sample (before the blank space)


and files should be left intact
