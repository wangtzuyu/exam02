import matplotlib.pyplot as plt

import numpy as np

import serial

import time



t = np.arange(0,10,0.1) # time vector; create Fs samples between 0 and 1.0 sec.

data=np.zeros(400)
x=np.zeros(100)
y=np.zeros(100)
z=np.zeros(100)
tilt=np.zeros(100)

serdev = '/dev/ttyACM0'

s = serial.Serial(serdev,115200)

for i in range(0, 400):
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    # print line
    data[i] = float(line)

for i in range (0,100):
    j=4*i
    x[i]=data[j]
    print(x[i])

for i in range (0,100):
    j=4*i+1
    y[i]=data[j]
    print(y[i])

for i in range (0,100):
    j=4*i+2
    z[i]=data[j]
    print(z[i])

for i in range (0,100):
    j=4*i+3
    tilt[i]=data[j]
    print(tilt[i])



fig, ax = plt.subplots(2, 1)

ax[0].plot(t,x)
ax[0].plot(t,y)
ax[0].plot(t,z)


ax[0].set_xlabel('Time')

ax[0].set_ylabel('ACC Vector')

ax[1].stem(t,tilt)
ax[1].set_xlabel('Time')

ax[1].set_ylabel('>5 cm or not')

plt.show()

s.close()