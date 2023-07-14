#!/usr/bin/python3
# 
import socketserver
import time
from datetime import datetime
import pytz
import requests

weekdays = ["Monday","Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "Sunday"]
tz = pytz.timezone('US/Eastern')

class MyTCPHandler(socketserver.StreamRequestHandler):

    def handle(self):
        print(f"{datetime.now()} Serving: {self.client_address[0]}")
        self.data = self.rfile.readline().strip()
        self.send_time()
        self.send_sun()
        self.send_weather()
        print(f"{datetime.now()} Served: {self.client_address[0]}")

    def send_time(self):
        #
        # Write out the current time, in the eastern time zone, suitable for setting the RTC of the pico
        dt = pytz.utc.localize(datetime.now()).astimezone(tz)
        newl = "\n".encode()
        self.wfile.write(str(dt.year).encode())
        self.wfile.write(newl)
        self.wfile.write(str(dt.month).encode())
        self.wfile.write(newl)
        self.wfile.write(str(dt.day).encode())
        self.wfile.write(newl)
        #
        # The Pico thinks dotw = 0 is Sunday, python things dotw = 0 is Monday!
        self.wfile.write(str((dt.weekday()+1)%7).encode())
        self.wfile.write(newl)
        self.wfile.write(str(dt.hour).encode())
        self.wfile.write(newl)
        self.wfile.write(str(dt.minute).encode())
        self.wfile.write(newl)
        self.wfile.write(str(dt.second).encode())
        self.wfile.write(newl)

    def send_weather(self):
        resp = requests.get("https://api.weather.gov/gridpoints/BOX/54,48/forecast")
        pop = 0
        high = 88
        low = 66
        if resp.status_code == 200:
            print(f"{datetime.now()} Got weather")
            dt = pytz.utc.localize(datetime.now()).astimezone(tz)
            tname = weekdays[(dt.weekday()+1)%7];
            for day in resp.json()['properties']['periods']:
                if day['name'] == tname:
                    pop = day['probabilityOfPrecipitation']['value']
                    if pop is None:
                        pop = 0
                    high = day['temperature']
                if day['name'] == f"{tname} Night":
                    low = day['temperature']
        else:
            print(f"{datetime.now()} Bad status code {resp.status_code} from weather API")

        newl = "\n".encode();
        self.wfile.write(str(pop).encode())
        self.wfile.write(newl)
        self.wfile.write(str(high).encode())
        self.wfile.write(newl)
        self.wfile.write(str(low).encode())
        self.wfile.write(newl)

    def send_sun(self):
        resp = requests.get("https://api.sunrise-sunset.org/json?lat=41.5273&long=-71.7807")
        if resp.status_code == 200:
            print(f"{datetime.now()} Got sunset")
            sunrise = resp.json()['results']['sunrise']
            sunset = resp.json()['results']['sunset']
        else:
            print(f"{datetime.now()} Bad status code {resp.status_code} from weather API")
            sunrise = "4:35"
            sunset = "7:45"

        newl = "\n".encode();
        self.wfile.write(sunrise[0:-6].encode())
        self.wfile.write(newl)
        self.wfile.write(sunset[0:-6].encode())
        self.wfile.write(newl)

if __name__ == "__main__":
    HOST, PORT = "0.0.0.0", 1030

    # Create the server, binding to localhost on port 9999
    with socketserver.TCPServer((HOST, PORT), MyTCPHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()
