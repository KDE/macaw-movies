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

#ifndef FETCHMETADATAQUERY_H
#define FETCHMETADATAQUERY_H

#include <QObject>

#include "Entities/Movie.h"

template<class T> class QList;
class QNetworkAccessManager;
class QNetworkReply;
class QString;

class Application;
class Movie;

/**
 * @brief The FetchMetadataQuery class
 *
 * @author Olivier CHURLAUD <olivier@churlaud.com>
 * @author Sébastien TOUZÉ <sebastien.touze@yahoo.fr>
 * @par Olivier CHURLAUD <olivier@churlaud.com>
 */
class FetchMetadataQuery : public QObject
{
    Q_OBJECT

public:
    explicit FetchMetadataQuery(QObject *parent = 0);
    ~FetchMetadataQuery();
    void sendInitRequest();
    void sendPrimaryRequest(QString title);
    void sendMovieRequest(int tmdbID);
    void sendPeopleRequest(int tmdbID);
    void sendPosterRequest(QString poster_path);
    bool isInitialized() { return m_initialized; }

signals:
    void primaryResponse(const QList<Movie>&);
    void movieResponse(const Movie&);
    void networkError(QString);
    void peopleResponse(const People&);

private slots:
    void on_initRequestResponse(QNetworkReply *reply);
    void on_primaryRequestResponse(QNetworkReply *reply);
    void on_movieRequestResponse(QNetworkReply *reply);
    void on_peopleRequestResponse(QNetworkReply *reply);
    void on_posterRequestResponse(QNetworkReply *reply);
    void slotError(int error);

private:
    QNetworkAccessManager *m_nmMovies;
    QNetworkAccessManager *m_nmPeople;
    QNetworkAccessManager *m_nmPosters;
    QString m_posterUrl;
    bool m_initialized;
    Application *m_app;
    Movie m_movie;
    People m_people;
};

#endif // FETCHMETADATAQUERY_H
