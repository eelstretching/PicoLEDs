#ifndef NTP_CLIENT_H
#define NTP_CLIENT_H

#pragma once

//
// Lat and long of the camp, for sunrise and sunset.
#define TR_LAT 41.527320
#define TR_LONG -71.780730

#define SUN_CLIENT_SERVER        "api.sunrise-sunset.org"
#define SUN_CLIENT_HTTP_REQUEST  "GET /json?lat=41.527320&long=-71.780730 HTTP/1.1\r\n" \
                                 "Host: " SUN_CLIENT_SERVER "\r\n" \
                                 "Connection: close\r\n" \
                                 "\r\n"
#define SUN_CLIENT_TIMEOUT_SECS  15



#endif