#!/usr/bin/python3
# 
import socketserver
import time
import requests

class MyTCPHandler(socketserver.StreamRequestHandler):

    def handle(self):
        self.data = self.rfile.readline().strip()
        # Likewise, self.wfile is a file-like object used to write back
        # to the client.
        self.send_time()
        self.send_sun()
        self.send_weather()
        print(f"served: {self.client_address[0]}")

    def send_time(self):
        #
        # Write out the current time, suitable for setting the RTC of the pico
        tm = time.localtime()
        newl = "\n".encode()
        self.wfile.write(str(tm.tm_year).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_mon).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_mday).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_wday).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_hour).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_min).encode())
        self.wfile.write(newl)
        self.wfile.write(str(tm.tm_sec).encode())
        self.wfile.write(newl)

    def send_weather(self):
        resp = requests.get("https://api.weather.gov/gridpoints/BOX/54,48/forecast")
        tomorrow = resp.json()['properties']['periods'][1]
        tomorrow_eve = resp.json()['properties']['periods'][2]
        pop = tomorrow['probabilityOfPrecipitation']['value']
        if pop is None:
            pop = 0
        high = tomorrow['temperature']
        low = tomorrow_eve['temperature']
        newl = "\n".encode();
        self.wfile.write(str(pop).encode())
        self.wfile.write(newl)
        self.wfile.write(str(high).encode())
        self.wfile.write(newl)
        self.wfile.write(str(low).encode())
        self.wfile.write(newl)

    def send_sun(self):
        resp = requests.get("https://api.sunrise-sunset.org/json?lat=41.5273&long=-71.7807")
        sunrise = resp.json()['results']['sunrise']
        sunset = resp.json()['results']['sunset']
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