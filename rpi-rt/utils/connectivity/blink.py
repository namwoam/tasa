import RPi.GPIO as GPIO
from time import sleep

# Needs to be BCM. GPIO.BOARD lets you address GPIO ports by periperal
# connector pin number, and the LED GPIO isn't on the connector

pin = 21

GPIO.setmode(GPIO.BCM)

# set up GPIO output channel
GPIO.setup(pin, GPIO.OUT)
while True:
    print("LED on")
    # On
    GPIO.output(pin, GPIO.LOW)

    # Wait a bit
    sleep(0.5)
    print("LED off")
    # Off
    GPIO.output(pin, GPIO.HIGH)

    sleep(0.5)