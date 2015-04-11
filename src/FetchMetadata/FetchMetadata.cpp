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

#include "FetchMetadata.h"

FetchMetadata::FetchMetadata(QObject *parent) :
    QObject(parent)
  , m_processState(false)
{
    Macaw::DEBUG("[FetchMetadata] Constructor");
    DatabaseManager *databaseManager = DatabaseManager::instance();

    m_fetchMetadataQuery = new FetchMetadataQuery(this);
    m_movieQueue = databaseManager->getMoviesNotImported();

    connect(this, SIGNAL(movieUpdated()),
            this, SLOT(startProcess()));

    Macaw::DEBUG("[FetchMetadata] Construction done");
}

FetchMetadata::~FetchMetadata()
{
    delete m_fetchMetadataQuery;
    Macaw::DEBUG("[FetchMetadata] Object destructed");
}

void FetchMetadata::startProcess()
{
    Macaw::DEBUG("[FetchMetadata] Start the process of metadata fetching");
    if (!m_movieQueue.isEmpty()) {
        m_movie = m_movieQueue.takeFirst();
        connect(m_fetchMetadataQuery, SIGNAL(primaryResponse(QList<Movie>&)),
                this, SLOT(processPrimaryResponse(QList<Movie>&)));
        connect(m_fetchMetadataQuery, SIGNAL(networkError(QString)),
                this, SLOT(networkError(QString)));

        QString l_cleanedTitle = cleanString(m_movie.title());
        m_fetchMetadataQuery->sendPrimaryRequest(l_cleanedTitle);
    } else {
        emit jobDone();
    }
}

QString FetchMetadata::cleanString(QString title)
{
    QRegExp l_sep("(\\_|\\-|\\ |\\,|\\)|\\(|\\]|\\[|\\.|\\!|\\?|\\#)");
    QStringList l_splittedTitle = title.split(l_sep, QString::SkipEmptyParts);

    QRegExp l_alphaOnly("(^['À-Ÿà-ÿA-Za-z]*$)");
    l_splittedTitle = l_splittedTitle.filter(l_alphaOnly);
    QString po;
    return l_splittedTitle.join(" ");
}

void FetchMetadata::processPrimaryResponse(QList<Movie> &movieList)
{
    Macaw::DEBUG("[FetchMetadata] Signal from primary request received");

    disconnect(m_fetchMetadataQuery, SIGNAL(primaryResponse(QList<Movie>&)),
            this, SLOT(processPrimaryResponse(QList<Movie>&)));

    QList<Movie> l_accurateList;

    if(movieList.count() == 1) {
        l_accurateList = movieList;
    } else if(movieList.count() > 1) {
        foreach(Movie l_movie, movieList) {
            if(cleanString(l_movie.title()).compare(cleanString(m_movie.title()), Qt::CaseInsensitive) == 0) {
                Macaw::DEBUG("[FetchMetadata] One title found");
                l_accurateList.push_back(l_movie);
            }
        }
    }

    if(l_accurateList.count() == 1) {
        Movie l_movie = l_accurateList.at(0);

        connect(m_fetchMetadataQuery, SIGNAL(movieResponse(Movie)),
                this, SLOT(processMovieResponse(Movie)));
        Macaw::DEBUG("[FetchMetadata] Movie request to be sent");
        m_fetchMetadataQuery->sendMovieRequest(l_movie.id());
    } else {
        if(l_accurateList.isEmpty()) {
            l_accurateList = movieList;
        }
        emit sendFetchMetadataDialog(m_movie, l_accurateList);
    }
}

void FetchMetadata::processMovieResponse(Movie receivedMovie)
{
    Macaw::DEBUG("[FetchMetadata] Signal from movie request received");
    DatabaseManager *databaseManager = DatabaseManager::instance();

    disconnect(m_fetchMetadataQuery, SIGNAL(movieResponse(Movie)),
            this, SLOT(processMovieResponse(Movie)));

    // Do not set the id since receivedMovie's id is from TMDB
    m_movie.setTitle(receivedMovie.title());
    m_movie.setOriginalTitle(receivedMovie.originalTitle());
    m_movie.setReleaseDate(receivedMovie.releaseDate());
    m_movie.setDuration(receivedMovie.duration());
    m_movie.setCountry(receivedMovie.country());
    m_movie.setSynopsis(receivedMovie.synopsis());
    m_movie.setPeopleList(receivedMovie.peopleList());
    m_movie.setColored(receivedMovie.isColored());
    m_movie.setPeopleList(receivedMovie.peopleList());
    m_movie.setPosterPath(receivedMovie.posterPath().right(receivedMovie.posterPath().size()-1));

    databaseManager->updateMovie(m_movie);
    emit movieUpdated();
}

void FetchMetadata::on_searchCanceled()
{
    m_processState = false;
    Macaw::DEBUG("[FetchMetadata] Dialog canceled");
    emit jobDone();
}

void FetchMetadata::on_selectedMovie(Movie movie)
{
    QList<Movie> l_movieList;
    l_movieList.append(movie);
    processPrimaryResponse(l_movieList);
}

void FetchMetadata::on_searchMovies(QString title)
{
    connect(m_fetchMetadataQuery, SIGNAL(primaryResponse(QList<Movie>&)),
            this, SLOT(processPrimaryResponseDialog(QList<Movie>&)));

    QString l_cleanedTitle = cleanString(title);
    m_fetchMetadataQuery->sendPrimaryRequest(l_cleanedTitle);
}

void FetchMetadata::processPrimaryResponseDialog(QList<Movie> &movieList)
{
    emit updateFetchMetadataDialog(movieList);
}

void FetchMetadata::networkError(QString error)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("Network error: " + error);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}
