/* Copyright (C) 2014 Macaw-Movies
 * (Olivier CHURLAUD, Sébastien TOUZÉ)
 *
 * This file is part of Macaw-Movies.
 *
 * Macaw-Movies is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Macaw-Movies is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Macaw-Movies.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PATHFORMOVIES_H
#define PATHFORMOVIES_H

#include <QString>

/**
 * @brief The PathForMovies class
 *
 * @author Olivier CHURLAUD <olivier@churlaud.com>
 */
class PathForMovies
{
public:
    explicit PathForMovies(QString path="", bool movies=true, bool shows=true);
    int id() const;
    void setId(const int id);
    QString path() const;
    void setPath(const QString path);
    bool hasMovies() const;
    void setMovies(const bool movies);
    bool hasShows() const;
    void setShows(const bool shows);
    int type() const;
    void setType(int type);

private:
    int m_id;
    QString m_path;
    int m_type;
};

#endif // PATHFORMOVIES_H
