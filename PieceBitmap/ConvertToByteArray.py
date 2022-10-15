from PIL import Image
from os import listdir
from os.path import isfile, join
import pathlib

#############################
# params
#############################

filename = "ChessFont25"
yAdvance = 25            # space between lines, used to center text vertically (and to write on multiple lines, but we don't)
invertColors = False      # invert if we use white background, i.e. we want to write the black pixels

#isRgb = False
#colorThreshold = 0.5

isRgb = True
colorThreshold = 128

startChar = 40  # if we start at 0, someone will be 10, and 10 is a newline character that gets skipped, so start higher to avoid that


#############################


filesInFolder = [f for f in listdir(filename) if isfile(join(filename, f))]
filesInFolder = [(filename + "/" + f) for f in filesInFolder if pathlib.Path(f).suffix == ".png"]
filesInFolder = sorted(filesInFolder)


file1 = "const uint8_t " + filename + "Bitmaps[] PROGMEM = {"
file2 = "const GFXglyph " + filename + "Glyphs[] PROGMEM = {\n"


nbGlyphs = 0;
byteOffset = 0;

firstGlyph = True
for f in filesInFolder:

    print(f)

    if firstGlyph:
        file1 += "\n"
    else:
        file1 += ",\n"

    nbGlyphs += 1
    firstByte = True
    
    im = Image.open(f, "r")
    w = im.width
    h = im.height
    
    if firstGlyph:
        file2 += "{"
    else:
        file2 += ",\n{"
        
    file2 += str(byteOffset) + ", "
    file2 += str(w) + ", "
    file2 += str(h) + ", "
    file2 += str(w) + ", "
    file2 += str(0) + ", "
    file2 += str(-h) + "}"
    
    data = im.getdata()
    iBit = 0
    byte = 0
    for p in data:
        if isRgb:
            val = p[0]
        else:
            val = p
    
        if invertColors:
            if val <= colorThreshold:
                byte += 1
        else:
            if val > colorThreshold:
                byte += 1
        
        if(iBit == 7):
            if not firstByte:
                file1 += ", "
            file1 += "0x{:02X}".format(byte)
            byteOffset += 1
            iBit = 0
            byte = 0
            firstByte = False
        else:
            byte = byte << 1
            iBit += 1
    if iBit > 0:
        # write residual byte data
        byte = byte << (7-iBit)
        if not firstByte:
            file1 += ", "
        file1 += "0x{:02X}".format(byte)
        byteOffset += 1
    
    firstGlyph = False
    


file3 = "const GFXfont " + filename + " PROGMEM = {(uint8_t *)" + filename + "Bitmaps, (GFXglyph *)" + filename + "Glyphs, " + str(startChar) + ", " + str(startChar + nbGlyphs-1) + ", " + str(yAdvance) + "};"

f = open(filename + "/" + filename + ".h", "w")
f.write(file1 + "};\n\n" + file2 + "\n};\n\n" + file3)
f.close()

