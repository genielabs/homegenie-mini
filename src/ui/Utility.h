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

#ifndef HOMEGENIE_MINI_UI_UTILITY_H
#define HOMEGENIE_MINI_UI_UTILITY_H

namespace UI {

    struct Point {
        float x, y;
    };

    class Utility {
    public:

        // Checks if a point is inside the bounds of a polygon
        // source: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
        static bool isInside(Point& point, const LinkedList<Point>& polygon) {
            int num_vertices = polygon.size();
            if (num_vertices < 3) return false;
            float x = point.x, y = point.y;
            bool inside = false;
            Point p1 = polygon[num_vertices - 1];
            for (int i = 0; i < num_vertices; i++) {
                Point p2 = polygon[i];
                if (p1.y == p2.y) {
                    if (p1.y == y && x >= std::min(p1.x, p2.x) && x <= std::max(p1.x, p2.x)) return true; // Check if it's on a side
                    p1 = p2;
                    continue;
                }
                if ( ((p1.y <= y && y < p2.y) || (p2.y <= y && y < p1.y)) &&
                     (x < (p2.x - p1.x) * (y - p1.y) / (p2.y - p1.y) + p1.x) ) {
                    inside = !inside;
                }
                p1 = p2;
            }
            return inside;
        }
    };

}

#endif //HOMEGENIE_MINI_UI_UTILITY_H
