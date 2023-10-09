#!/usr/bin/env python

import matplotlib.image as mpimage

img = mpimage.imread("/home/cowley/Pictures/maze.png")
rows = len(img)
cols = len(img[0])


def isfilled(val):
    return all(x > 0.2 for x in val)


def getval(img, x, y):
    if y < 0 or y >= len(img):
        return False
    if x < 0 or x >= len(img[y]):
        return False
    return isfilled(img[y][x])


segments = []
for iy in range(-1, rows + 1):
    seg_start = None
    for ix in range(-1, cols + 1):
        if getval(img, ix, iy) and not getval(img, ix, iy - 1):
            if seg_start is None:
                seg_start = (ix - 0.5, iy - 0.5)
        else:
            if seg_start is not None:
                segments.append((seg_start, (ix - 0.5, iy - 0.5), (0.0, 0.0, 1.0)))
                seg_start = None
    if seg_start is not None:
        segments.append((seg_start, (cols - 0.5, iy - 0.5), (0.0, 0.0, 1.0)))


for iy in range(-1, rows + 1):
    seg_start = None
    for ix in range(-1, cols + 1):
        if getval(img, ix, iy) and not getval(img, ix, iy + 1):
            if seg_start is None:
                seg_start = (ix - 0.5, iy + 0.5)
        else:
            if seg_start is not None:
                segments.append((seg_start, (ix - 0.5, iy + 0.5), (0.0, 0.0, -1.0)))
                seg_start = None
    if seg_start is not None:
        segments.append((seg_start, (cols - 0.5, iy + 0.5), (0.0, 0.0, -1.0)))

for ix in range(-1, cols + 1):
    seg_start = None
    for iy in range(-1, rows + 1):
        if getval(img, ix, iy) and not getval(img, ix - 1, iy):
            if seg_start is None:
                seg_start = (ix - 0.5, iy - 0.5)
        else:
            if seg_start is not None:
                segments.append((seg_start, (ix - 0.5, iy - 0.5), (-1.0, 0.0, 0.0)))
                seg_start = None
    if seg_start is not None:
        segments.append((seg_start, (ix - 0.5, rows - 0.5), (-1.0, 0.0, 0.0)))

for ix in range(-1, cols + 1):
    seg_start = None
    for iy in range(-1, rows + 1):
        if getval(img, ix, iy) and not getval(img, ix + 1, iy):
            if seg_start is None:
                seg_start = (ix + 0.5, iy - 0.5)
        else:
            if seg_start is not None:
                segments.append((seg_start, (ix + 0.5, iy - 0.5), (1.0, 0.0, 0.0)))
                seg_start = None
    if seg_start is not None:
        segments.append((seg_start, (ix + 0.5, rows - 0.5), (1.0, 0.0, 0.0)))

print(segments[:10])

res = {}
res["players"] = []
player = {
    "y": 2.5,
    "x": 845.0,
    "z": -5.0,
    "yaw": 270.0,
}
res["players"].append(player)
res["surfaces"] = []
for segment in segments:
    p1 = {"x": segment[0][0], "z": -segment[0][1], "y": 0.0}
    p2 = {
        "x": segment[1][0],
        "z": -segment[1][1],
        "y": 0.0,
    }
    p3 = {
        "x": segment[1][0],
        "z": -segment[1][1],
        "y": 5.0,
    }
    p4 = {
        "x": segment[0][0],
        "z": -segment[0][1],
        "y": 5.0,
    }
    surface = [p1, p2, p3, p4, segment[2]]
    res["surfaces"].append(surface)

xmin = 0.0
xmax = 0.0
zmin = 0.0
zmax = 0.0

with open("maze.txt", "w") as f:
    f.write(f"{player['x']} {player['y']} {player['z']} {player['yaw']}\n")
    f.write(f"{len(res['surfaces'])}\n")
    for segment in res["surfaces"]:
        for point in segment[:4]:
            if point["x"] < xmin:
                xmin = point["x"]
            if point["x"] > xmax:
                xmax = point["x"]
            if point["z"] < zmin:
                zmin = point["z"]
            if point["z"] > zmax:
                zmax = point["x"]
        point = segment[0]
        norm = f"{segment[4][0]} {segment[4][1]} {segment[4][2]}"
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        point = segment[1]
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        point = segment[2]
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        point = segment[3]
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        point = segment[2]
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        point = segment[0]
        f.write(f"{point['x']} {point['y']} {point['z']} {norm}\n")
        f.write("\n")
    f.write(f"{xmin} 0.0 {zmin} 0.0 1.0 0.0\n")
    f.write(f"{xmax} 0.0 {zmin} 0.0 1.0 0.0\n")
    f.write(f"{xmin} 0.0 {zmax} 0.0 1.0 0.0\n")
    f.write(f"{xmax} 0.0 {zmax} 0.0 1.0 0.0\n")
    f.write(f"{xmin} 0.0 {zmax} 0.0 1.0 0.0\n")
    f.write(f"{xmax} 0.0 {zmin} 0.0 1.0 0.0\n")
