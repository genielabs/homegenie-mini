/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
 *
 *
 * This file is part of HomeGenie-Mini (HGM).
 *
 *  HomeGenie-Mini is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HomeGenie-Mini is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HomeGenie-Mini.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Authors:
 * - Generoso Martello <gene@homegenie.it>
 *
 */

#ifndef HOMEGENIE_MINI_UTILITY_H
#define HOMEGENIE_MINI_UTILITY_H

namespace UI {

    struct Point {
        float x, y;
    };

    class Utility {
    public:

        // Checks if a point is inside the bounds of a polygon
        // source: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
        static bool isInside(Point point, LinkedList<Point> polygon) {
            int num_vertices = polygon.size();
            if (num_vertices == 0) return false;

            float x = point.x, y = point.y;
            bool inside = false;

            // Store the first point in the polygon and initialize
            // the second point
            Point p1 = polygon[0], p2;

            // Loop through each edge in the polygon
            for (int i = 1; i <= num_vertices; i++) {
                // Get the next point in the polygon
                p2 = polygon[i % num_vertices];

                // Check if the point is above the minimum y
                // coordinate of the edge
                if (y > min(p1.y, p2.y)) {
                    // Check if the point is below the maximum y
                    // coordinate of the edge
                    if (y <= max(p1.y, p2.y)) {
                        // Check if the point is to the left of the
                        // maximum x coordinate of the edge
                        if (x <= max(p1.x, p2.x)) {
                            // Calculate the x-intersection of the
                            // line connecting the point to the edge
                            float x_intersection
                                    = (y - p1.y) * (p2.x - p1.x)
                                      / (p2.y - p1.y)
                                      + p1.x;

                            // Check if the point is on the same
                            // line as the edge or to the left of
                            // the x-intersection
                            if (p1.x == p2.x
                                || x <= x_intersection) {
                                // Flip the inside flag
                                inside = !inside;
                            }
                        }
                    }
                }

                // Store the current point as the first point for
                // the next iteration
                p1 = p2;
            }

            // Return the value of the inside flag
            return inside;
        }
    };

}

#endif //HOMEGENIE_MINI_UTILITY_H
