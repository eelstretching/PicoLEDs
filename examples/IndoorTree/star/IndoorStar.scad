// A star for the top of the indoor tree, made to hold a piece of seed pixel
// strand: three nested star-shaped rings of channel on the front, pressed full
// of the strand, with the pixels landing on every tip and valley.
//
// The rings are sized from the strand's pixel spacing (1 inch), so each edge
// of a ring is a whole number of pixel gaps: 3, 2 and 1 inch edges give rings
// of 30, 20 and 10 pixel positions. The bottom valley of each ring is left
// empty: that's where the strand dives through a hole to the back, hops in to
// the next ring in a pocket on the back (three pixels and some slack stay
// hidden there for each hop), and comes up again. So the star shows
// 29 + 19 + 9 = 57 pixels and takes 63 off the strand. IndoorStar.h knows
// this layout.
//
// Parts (pick one with -D part=...):
//   "star"     the star itself, printed flat, channels up. No supports.
//   "diffuser" an optional cap that slips over the front, printed face down
//              in white or natural PLA/PETG, for a soft glowing star instead
//              of points of light.
//   "both"     the two side by side, for a picture.
//
//   openscad -D 'part="star"' -o IndoorStar.stl IndoorStar.scad
//   openscad -D 'part="diffuser"' -o IndoorStarDiffuser.stl IndoorStar.scad
//
// The star is 214 mm across and, with the tongue below it that zip ties to
// the tree's top leader, 242 mm tall: it fits a 250 x 220 mm bed (Prusa CORE
// One) or 256 x 256 (Bambu), lengthwise. Shorten tongue_len for a smaller
// bed. A groove up the back of the tongue carries the feed wire.
//
// Measure a pixel on your strand before printing: the channel has to take
// the pixel bead snugly. A dab of hot glue or clear silicone holds the rest.

part = "both";

// The spacing of the pixels along the strand, in mm.
pitch = 25.4;
// Edge length of each ring, in pixel gaps, outside in.
edges = [3, 2, 1];
// Valley radius over tip radius. 0.382 is a classic five pointed star.
k = 0.382;

// The channel for the strand.
channel_w = 6.5;
channel_d = 4.5;
// The plate the channels are cut into.
plate_t = 9;
// How far the plate goes past the outer channel.
margin = 4;
// Holes where the strand goes through to the back.
hole_d = 7.5;
// Pocket on the back for the hops between rings and the hidden pixels.
pocket_d = 3;
pocket_r = 9;

// Mounting tongue.
tongue_w = 26;
tongue_len = 90;
tie_slot = [3.2, 5.5];
feed_w = 5;
feed_d = 3;

// Diffuser cap.
diff_gap = 14;     // from the pixels to the inside of the cap's front
diff_t = 1.2;      // front face
diff_wall = 1.6;
diff_clear = 0.35;
diff_overlap = 7;  // how far it slides over the plate's edge

$fn = 48;

// Tip radius of a ring whose edges are n pixel gaps long.
function edge_over_r() = sqrt(1 + k * k - 2 * k * cos(36));
function ring_r(n) = n * pitch / edge_over_r();

// A five pointed star of tip radius r, one tip straight up.
function star_pts(r) = [for (i = [0 : 9])
    let (a = 90 + i * 36, rr = (i % 2 == 0) ? r : k * r) [rr * cos(a), rr * sin(a)]];

module star2d(r) { polygon(star_pts(r)); }

module ring_channel(r) {
    difference() {
        offset(r = channel_w / 2) star2d(r);
        offset(delta = -channel_w / 2) star2d(r);
    }
}

R0 = ring_r(edges[0]);

// Where each ring's bottom valley is.
function valley_y(i) = -k * ring_r(edges[i]);

module outline2d() {
    offset(r = channel_w / 2 + margin) star2d(R0);
    translate([-tongue_w / 2, valley_y(0) - tongue_len]) square([tongue_w, tongue_len]);
}

module star_part() {
    difference() {
        linear_extrude(plate_t) outline2d();
        // Front channels.
        for (n = edges)
            translate([0, 0, plate_t - channel_d]) linear_extrude(channel_d + 1) ring_channel(ring_r(n));
        // Holes at the bottom valley of every ring.
        for (i = [0 : len(edges) - 1])
            translate([0, valley_y(i), -1]) cylinder(d = hole_d, h = plate_t + 2);
        // Back pocket joining the holes.
        translate([0, 0, -1]) linear_extrude(pocket_d + 1) hull() {
            translate([0, valley_y(0)]) circle(r = pocket_r);
            translate([0, valley_y(len(edges) - 1)]) circle(r = pocket_r);
        }
        // Feed groove up the back of the tongue to the outer hole.
        translate([-feed_w / 2, valley_y(0) - tongue_len - 1, -1])
            cube([feed_w, tongue_len + 1, feed_d + 1]);
        // Zip tie slots, three pairs.
        for (j = [0 : 2], s = [-1, 1])
            translate([s * (tongue_w / 2 - 5.5) - tie_slot[0] / 2,
                       valley_y(0) - 30 - j * 25, -1])
                cube([tie_slot[0], tie_slot[1], plate_t + 2]);
    }
}

module diffuser_part() {
    inner = channel_w / 2 + margin + diff_clear;
    h = diff_t + diff_gap + diff_overlap;
    difference() {
        linear_extrude(h) offset(r = inner + diff_wall) star2d(R0);
        translate([0, 0, diff_t]) linear_extrude(h) offset(r = inner) star2d(R0);
        // Notch for the tongue.
        translate([-(tongue_w / 2 + diff_clear), valley_y(0) - 80, diff_t + diff_gap])
            cube([tongue_w + 2 * diff_clear, 80, h]);
    }
}

if (part == "star") star_part();
else if (part == "diffuser") diffuser_part();
else {
    star_part();
    translate([2 * R0 + 30, 0, 0]) diffuser_part();
}

echo(str("Rings: tip radius ", [for (n = edges) ring_r(n)], " mm, ",
         [for (n = edges) 10 * n], " pixel positions"));
