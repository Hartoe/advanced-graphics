import random
import math

file = open("scenes/scene_3_angle_1.trace", "a")
# Scene 2:
# for i in range(10000):
#     x = random.uniform(-1.0, 1.0)# * random.uniform(-1.0, 1.0)
#     y = random.uniform(-1.0, 1.0)# * random.uniform(-1.0, 1.0)
#     z = random.uniform(-1.0, 1.0)# * random.uniform(-1.0, 1.0)
#     file.write("SPHERE ({} {} {}) 0.02 (LAM {} {} {})\n".format(x, y, z, x, y, z))

# Scene 2:
a = 0.2
n = 50
for v in [(x,y,z) for x in [-1.0,1.0] for y in [-1.0, 1.0] for z in [-1.0, 1.0]]:
    (x, y, z) = v
    x = x * math.cos(a) - z * math.sin(a)
    z = x * math.sin(a) + z * math.cos(a)
    for i in range(n):
        if i == 0:
            continue
        xx = (x*i/n)
        yy = (y*i/n)
        zz = (z*i/n)
        file.write("SPHERE ({} {} {}) 0.02 (LAM {} {} {})\n".format(xx, yy, zz, xx, yy, zz))
