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

#ifndef SERVICESMANAGER_H
#define SERVICESMANAGER_H

#include <QObject>

#include "DatabaseManager.h"
#include "Entities/Movie.h"

class DatabaseManager;
class Movie;

/**
 * @brief The ServicesManager class
 * @author Olivier CHURLAUD <olivier@churlaud.com>
 */
class ServicesManager : public QObject
{
    Q_OBJECT
public:
    explicit ServicesManager(QObject *parent = 0);
    static ServicesManager* instance();
    QList<Movie> matchingMovieList() const { return m_matchingMovieList; }
    void setMatchingMovieList(const QString pattern, bool shows);
    bool toWatchState() const { return m_toWatchState; }
    void setToWatchState(const bool state) { m_toWatchState = state; }
    DatabaseManager* databaseManager() { return m_databaseManager; }

signals:
    void requestPannelsUpdate();
    void requestTempStatusBarMessage(QString message, int time = 0);

public slots:
    void pannelsUpdate();
    void showTempStatusBarMessage(QString message, int time);

private:
    QList<Movie> m_matchingMovieList;
    DatabaseManager *m_databaseManager;
    bool m_toWatchState;
};

#endif // SERVICESMANAGER_H
