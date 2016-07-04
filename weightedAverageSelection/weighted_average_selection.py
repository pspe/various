



def getCount (categories):
    total = float (sum (categories))
    avg = 0.0
    for cat in categories:
        avg += cat * (cat / total)
    return avg




samples = [
    [10,8,9,5,11,15],
    [10,8,9,5,100,15],
    [0,0,0,0,3,1,100],
    [0,0,0,0,0,0,0,0,0,0,0,3,1,30000]
    ]

for smpl in samples:
    avg = getCount (set (smpl))
    print ("Avg: ",avg,"  ",smpl)

    
