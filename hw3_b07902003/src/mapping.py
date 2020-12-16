import sys
Mapfile = open(sys.argv[1] , "r+" , encoding = "big5-hkscs")
output = open(sys.argv[2] , "w+" , encoding = "big5-hkscs")
zhuyin = []
for i in range(37):
    zhuyin.append([])
for line in Mapfile:
    word_sound = line.split( )
    sound = word_sound[1].split('/')
    for s in sound:
        ind = ord(s[0]) - ord('ㄅ')
        # print(word_sound[0] , ind , zhuyin[ind])
        if word_sound[0] not in zhuyin[ind]:
            zhuyin[ind].append(word_sound[0])
    zhuyin.append(word_sound[0])
# print(ord('ㄅ'))
for i in range(len(zhuyin)):
    if i < 37:
        c = chr(12549 + i)
        output.write(c)
        for word in zhuyin[i]:
            output.write(" " + word)
        output.write("\n")
    else:
        output.write(zhuyin[i] + " " + zhuyin[i] + "\n")
Mapfile.close()
output.close()