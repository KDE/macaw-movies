/* Copyright (C) 2014 Macaw-Movies
 * (Olivier CHURLAUD, Sébastien TOUZÉ)
 *
 * This file is part of Macaw-Movies.
 *
 * Macaw-Movies is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Macaw-Movies is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Macaw-Movies.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DatabaseManager.h"

#include <QApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include "include_var.h"

#include "MacawDebug.h"
#include "Entities/Episode.h"
#include "Entities/Movie.h"
#include "Entities/PathForMovies.h"
#include "Entities/Playlist.h"
#include "Entities/Show.h"

/**
 * @brief Constructor.
 * Opens the Database. If empty, create the schema.
 */
DatabaseManager::DatabaseManager()
{
    m_movieFields = "m.id, "
                    "m.title, "
                    "m.original_title, "
                    "m.release_date, "
                    "m.country, "
                    "m.duration, "
                    "m.synopsis, "
                    "m.id_path, "
                    "m.file_path, "
                    "m.poster_path, "
                    "m.colored, "
                    "m.format, "
                    "m.suffix, "
                    "m.rank, "
                    "m.imported, "
                    "m.id_tmdb, "
                    "m.show ";

    m_episodeFields = "e.id, "
                      "e.number, "
                      "e.season, "
                      "e.id_show, "
                      "e.id_movie ";

    m_showFields =  "s.id, "
                    "s.name, "
                    "s.finished ";

    m_peopleFields = "p.id, "
                     "p.name, "
                     "p.birthday, "
                     "p.biography, "
                     "p.imported, "
                     "p.id_tmdb ";

    m_tagFields = "t.id, "
                  "t.name ";

    openDB();
    createTables();
    Macaw::DEBUG("[DatabaseManager] object created");
}

/**
 * @brief Hydrates a movie from the database and all the corresponding lists
 *
 * @param QSqlQuery containing the data
 * @return Movie hydrated object
 */
Movie DatabaseManager::hydrateMovie(QSqlQuery &query)
{
    Movie l_movie = hydrateMovieOnly(query);
    setTagsToMovie(l_movie);
    setPeopleToMovie(l_movie);

    return l_movie;
}

/**
 * @brief Hydrates a movie from the database
 *
 * @param QSqlQuery containing the data
 * @return Movie hydrated object
 */
Movie DatabaseManager::hydrateMovieOnly(QSqlQuery &query)
{
    Movie l_movie;
    l_movie.setId(query.value(0).toInt());
    l_movie.setTitle(query.value(1).toString());
    l_movie.setOriginalTitle(query.value(2).toString());
    l_movie.setReleaseDate(QDate::fromString(query.value(3).toString(), DATE_FORMAT));
    l_movie.setCountry(query.value(4).toString());
    l_movie.setDuration(QTime::fromMSecsSinceStartOfDay(query.value(5).toInt()));
    l_movie.setSynopsis(query.value(6).toString());
    l_movie.setFileAbsolutePath(getMoviesPathById(query.value(7).toInt())
                                +QDir::separator()
                                +query.value(8).toString());
    l_movie.setFileRelativePath(query.value(8).toString());
    l_movie.setPosterPath(query.value(9).toString());
    l_movie.setColored(query.value(10).toBool());
    l_movie.setFormat(query.value(12).toString());
    l_movie.setSuffix(query.value(13).toString());
    l_movie.setRank(query.value(14).toInt());
    l_movie.setImported(query.value(14).toBool());
    l_movie.setTmdbId(query.value(15).toInt());
    l_movie.setShow(query.value(16).toBool());

    return l_movie;
}

/**
 * @brief Hydrates an episode (from a show) from the database
 *
 * @param QSqlQuery containing the data
 * @return Episode hydrated object
 */
Episode DatabaseManager::hydrateEpisode(QSqlQuery &query)
{
    Episode l_episode;
    l_episode.setId(query.value(0).toInt());
    l_episode.setNumber(query.value(1).toInt());
    l_episode.setSeason(query.value(2).toInt());

    Show l_show;
    l_show.setId(query.value(5).toInt());
    l_show.setName(query.value(6).toString());
    l_show.setFinished(query.value(7).toBool());
    l_episode.setShow(l_show);

    setMovieToEpisode(l_episode);

    return l_episode;
}

/**
 * @brief Hydrates an episode (from Show) from the database, knowing the movie
 *
 * @param QSqlQuery containing the data
 * @return Episode hydrated object
 */
Episode DatabaseManager::hydrateEpisode(QSqlQuery &query, const Movie &movie)
{
    Episode l_episode;
    l_episode.setId(query.value(0).toInt());
    l_episode.setNumber(query.value(1).toInt());
    l_episode.setSeason(query.value(2).toInt());

    Show l_show;
    l_show.setId(query.value(5).toInt());
    l_show.setName(query.value(6).toString());
    l_show.setFinished(query.value(7).toBool());
    l_episode.setShow(l_show);

    l_episode.setMovie(movie);

    return l_episode;
}

/**
 * @brief Hydrates Show from the database
 *
 * @param QSqlQuery containing the data
 * @return Show hydrated object
 */
Show DatabaseManager::hydrateShow(QSqlQuery &query)
{
    Show l_show;
    l_show.setId(query.value(0).toInt());
    l_show.setName(query.value(1).toString());
    l_show.setFinished(query.value(2).toBool());

    return l_show;
}

/**
 * @brief Hydrates a person from the database
 *
 * @param QSqlQuery containing the data
 * @return People hydrated object
 */
People DatabaseManager::hydratePeople(QSqlQuery &query)
{
    People l_people;
    l_people.setId(query.value(0).toInt());
    l_people.setName(query.value(1).toString());
    l_people.setBirthday(QDate::fromString(query.value(2).toString(), DATE_FORMAT));
    l_people.setBiography(query.value(3).toString());
    l_people.setImported(query.value(4).toBool());
    l_people.setTmdbId(query.value(5).toInt());

    if (query.value(6).isValid())
    {
        l_people.setType(query.value(6).toInt());
    }

    return l_people;
}

/**
 * @brief Hydrates a tag from the database
 *
 * @param QSqlQuery containing the data
 * @return Tag hydrated object
 */
Tag DatabaseManager::hydrateTag(QSqlQuery &query)
{
    Tag l_tag;
    l_tag.setId(query.value(0).toInt());
    l_tag.setName(query.value(1).toString());

    return l_tag;
}

/**
 * @brief Hydrates a playlist from the database
 *
 * @param QSqlQuery containing the data
 * @return Playlist hydrated object
 */
Playlist DatabaseManager::hydratePlaylist(QSqlQuery &query)
{
    Playlist l_playlist;
    l_playlist.setId(query.value(0).toInt());
    l_playlist.setName(query.value(1).toString());
    l_playlist.setRate(query.value(2).toInt());
    l_playlist.setCreationDate(QDateTime::fromTime_t(query.value(3).toInt()));
    setMoviesToPlaylist(l_playlist);

    return l_playlist;
}

/**
 * @brief Opens (or create the file of) the database
 *
 * @return bool db.open()
 */
bool DatabaseManager::openDB()
{
    Macaw::DEBUG("[DatabaseManager] openDB");
    if (QSqlDatabase::contains("Movies-database"))
    {
        m_db = QSqlDatabase::database("Movies-database");
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "Movies-database");
    }

    QString l_dbPath = qApp->property("filesPath").toString() + "database.sqlite";
    m_db.setDatabaseName(l_dbPath);

    if (!m_db.open()) {
        return false;
    }

    QSqlQuery l_query(m_db);
    l_query.prepare("PRAGMA foreign_keys = ON");

    if (!l_query.exec()) {
        Macaw::DEBUG("In openDB:");
        Macaw::DEBUG(l_query.lastError().text());
        return false;
    }

    return true;
}

/**
 * @brief Closes the database.
 *
 * @return bool
 */
bool DatabaseManager::closeDB()
{
    Macaw::DEBUG("[DatabaseManager] Close database");
    m_db.close();

    return true;
}

/**
 * @brief Deletes the database.
 *
 * @return bool
 */
bool DatabaseManager::deleteDB()
{
    Macaw::DEBUG("[DatabaseManager] deleteDB");
    closeDB();

    return QFile::remove(m_db.databaseName());
}

/**
 * @brief Upgrades DB between diffent DB versions
 * This function needs to be updated each time a change to the DB structure is made
 *
 * @return bool
 */
bool DatabaseManager::upgradeDB(int fromVersion, int toVersion)
{
    Macaw::DEBUG_IN("[DatabaseManager] upgradeDB");
    bool  l_ret = false;
    int l_fromVersion(fromVersion);

    if (m_db.isOpen())
    {
        // We need to have a clean version of the db instance.
        //There might be a better way to do this.
        m_db.close();

        Macaw::DEBUG_IN("[DatabaseManager] backup database");
        QFile::copy(m_db.databaseName(),
                    m_db.databaseName()+"_backup"
                                       + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

        Macaw::DEBUG_OUT("[DatabaseManager] database backup done");

        m_db.open();

        QSqlQuery l_query(m_db);

        // PRAGMA is disabled so that renaming and deleting a database don't act on cascade
        l_ret = l_query.exec("PRAGMA foreign_keys = OFF");

        //switch to DB_VERSION 050
        if (toVersion >= 50) {

            Macaw::DEBUG_IN("[DatabaseManager] upgrade to v050");
            l_query.finish();
            l_query.clear();

            if (!m_db.record("config").contains("media_player")) {
                l_ret &= l_query.exec("ALTER TABLE config ADD media_player VARCHAR(255)");
                if(!l_ret)
                {
                    Macaw::DEBUG(l_query.lastError().text());
                }
            }

            if (!m_db.record("movies").contains("id_tmdb")) {
                l_ret &= l_query.exec("ALTER TABLE movies ADD id_tmdb INTEGER");
                l_ret &= l_query.exec("UPDATE movies SET id_tmdb = 0");
                if(!l_ret)
                {
                    Macaw::DEBUG(l_query.lastError().text());
                }
            }

            if (!m_db.record("movies").contains("show")) {
                l_ret &= l_query.exec("ALTER TABLE movies ADD show BOOLEAN");
                l_ret &= l_query.exec("UPDATE movies SET show = 0");
                if(!l_ret)
                {
                    Macaw::DEBUG(l_query.lastError().text());
                }
            }
            if (!m_db.tables().contains("path_list")) {
                Macaw::DEBUG_IN("[DatabaseManager] upgrade path_list table");
                l_ret &= createTablePathList(l_query);
                l_ret &= l_query.exec("INSERT INTO path_list(id, movies_path, imported) SELECT id, movies_path, imported FROM paths_list");
                l_ret &= l_query.exec("DROP TABLE paths_list");
                l_ret &= l_query.exec("UPDATE path_list SET type = 1");
                if(!l_ret)
                {
                    Macaw::DEBUG(l_query.lastError().text());
                }
                Macaw::DEBUG_OUT("[DatabaseManager] upgrade path_list table finished");
            }

            if (!m_db.record("people").contains("id_tmdb")) {
                Macaw::DEBUG_IN("[DatabaseManager] upgrade people table");
                l_ret &= l_query.exec("ALTER TABLE people ADD id_tmdb INTEGER");
                l_ret &= l_query.exec("ALTER TABLE people ADD imported BOOLEAN");
                l_ret &= l_query.exec("UPDATE people SET imported = 0, id_tmdb = 0");
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }
            }

            if (!m_db.record("movies").contains("id_path")) {
                Macaw::DEBUG_IN("[DatabaseManager] upgrade movies table");
                l_ret &= l_query.exec("ALTER TABLE movies ADD id_path INTEGER");
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }
                l_ret &= l_query.exec("ALTER TABLE movies RENAME TO movies_old");
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }
                l_ret &= createTableMovies(l_query);
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }
                l_ret &= l_query.exec("UPDATE movies_old SET id_path=1");
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }
                l_ret &= l_query.exec("INSERT INTO movies SELECT "+ m_movieFields +"FROM movies_old AS m");
                if(!l_ret){
                    Macaw::DEBUG("Copying table movies failed");
                    Macaw::DEBUG(l_query.lastError().text());
                }
                l_ret &= l_query.exec("DROP TABLE movies_old");
                if(!l_ret){
                    Macaw::DEBUG(l_query.lastError().text());
                }

                l_query.exec("SELECT id, movies_path FROM path_list");
                QStringList l_pathList;
                QList<int> l_idList;
                while (l_query.next())
                {
                    l_idList.append(l_query.value(0).toInt());
                    l_pathList.append(l_query.value(1).toString());
                }
                l_query.exec("SELECT file_path FROM movies");

                while (l_query.next())
                {
                    for (int i = 0 ; i < l_pathList.count() ; i++) {
                        if (l_query.value(0).toString().startsWith(l_pathList.at(i))) {
                            QString l_moviePath = l_query.value(0).toString();
                            QString l_moviePath_new = l_moviePath;
                            l_moviePath_new.remove(0, l_pathList.at(i).count()+1);
                            QSqlQuery l_query2(m_db);
                            l_ret &= l_query2.exec("UPDATE movies "
                                                   "SET file_path='"+l_moviePath_new+"', "+
                                                   "id_path="+QString::number(l_idList.at(i))+' '+
                                                   "WHERE file_path='"+l_moviePath+'\'');
                            if(!l_ret){
                                Macaw::DEBUG(l_query2.lastError().text());
                            }
                        }
                    }
                }
                Macaw::DEBUG_OUT("[DatabaseManager] upgrade movies table finished");
            }

            l_ret &= createTableShow(l_query);
            l_ret &= createTableEpisodes(l_query);

            if(l_ret) {
                l_ret &= l_query.exec("UPDATE config "
                                      "SET db_version = 050");
                l_fromVersion = 050;
            } else {
                m_db.close();

                Macaw::DEBUG_IN("[DatabaseManager] FAILED => Come back to backup");
                QDir l_backups = m_db.databaseName();
                l_backups.cdUp();
                QStringList l_backupNameList = l_backups.entryList(QDir::Files|QDir::NoDotAndDotDot,
                                                                   QDir::Name);
                Macaw::DEBUG("Return to "+l_backupNameList.last());

                this->deleteDB();
                QFile::copy(l_backups.absolutePath()+QDir::separator()+l_backupNameList.last(),
                            m_db.databaseName());

                Macaw::DEBUG_OUT("[DatabaseManager] Returned to backup");

                this->openDB();
            }
            Macaw::DEBUG_OUT("[DatabaseManager] exits upgrade to v050");
        }
    }
    Macaw::DEBUG_OUT("[DatabaseManager] exits upgradeDB");

    return l_ret;
}

/**
 * @brief Creates all the tables
 *
 * @return bool
 */
bool DatabaseManager::createTables()
{
    Macaw::DEBUG("[DatabaseManager] createTables");
    bool l_ret = false;

    if (m_db.isOpen())
    {
        QSqlQuery l_query(m_db);
        l_ret &= createTablePeople(l_query);

        if(m_db.tables().contains("config"))
        {
            Macaw::DEBUG("[DatabaseManager.createTable] config table exists");
            l_query.exec("SELECT db_version FROM config");
            l_query.next();
            if(l_query.value(0).toInt() != DB_VERSION)
            {
                l_ret = upgradeDB(l_query.value(0).toInt(), DB_VERSION);
            }
        }
        else    //if config table do not exists then the db is empty...
        {
            Macaw::DEBUG("[DatabaseManager.createTable] configTable does not exist");

            l_ret = l_query.exec("PRAGMA foreign_keys = ON");

            l_ret &= createTableMovies(l_query);
            l_ret &= createTablePeople(l_query);
            l_ret &= createTableMoviesPeople(l_query);
            l_ret &= createTablePlaylists(l_query);
            l_ret &= createTableMoviesPlaylists(l_query);
            l_ret &= createTableTags(l_query);
            l_ret &= createTableMoviesTags(l_query);
            l_ret &= createTableShow(l_query);
            l_ret &= createTableEpisodes(l_query);
            l_ret &= createTablePathList(l_query);
            if (l_ret) {
                l_ret &= createTableConfig(l_query);
            }
        }
    }

    return l_ret;
}

/**
 * @brief Create the table `movies`
 * @param query
 * @return
 */
bool DatabaseManager::createTableMovies(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS movies("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "title VARCHAR(255) NOT NULL, "
                  "original_title VARCHAR(255), "
                  "release_date VARCHAR(10), "
                  "country VARCHAR(50), "
                  "duration INTEGER, "
                  "synopsis TEXT, "
                  "id_path INTEGER NOT NULL, "
                  "file_path VARCHAR(255) NOT NULL, "
                  "poster_path VARCHAR(255), "
                  "colored BOOLEAN, "
                  "format VARCHAR(10), "
                  "suffix VARCHAR(10), "
                  "rank INTEGER, "
                  "imported BOOLEAN, "
                  "id_tmdb INTEGER, "
                  "show BOOLEAN, "
                  "UNIQUE (id_path, file_path) ON CONFLICT IGNORE "
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableMovies:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create the table `people`
 * @param query
 * @return
 */
bool DatabaseManager::createTablePeople(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS people("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "name VARCHAR(200) NOT NULL, "
                  "birthday VARCHAR(10), "
                  "biography TEXT, "
                  "imported BOOLEAN, "
                  "id_tmdb INTEGER"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTablePeople:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}
/**
 * @brief Create the table `movies_people` which links between people and movies (a type of person is given here)
 * @param query
 * @return
 */
bool DatabaseManager::createTableMoviesPeople(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS movies_people("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "id_movie INTEGER NOT NULL, "
                  "id_people INTEGER NOT NULL, "
                  "type INTEGER NOT NULL, "
                  "UNIQUE (id_people, id_movie, type) ON CONFLICT IGNORE, "
                  "FOREIGN KEY(id_movie) REFERENCES movies ON DELETE CASCADE, "
                  "FOREIGN KEY(id_people) REFERENCES people ON DELETE CASCADE"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableMoviesPeople:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create the table `playlists` and the "To Watch" default playlist
 * @param query
 * @return
 */
bool DatabaseManager::createTablePlaylists(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS playlists("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "name VARCHAR(255) UNIQUE NOT NULL, "
                  "rate INTEGER, "
                  "creation_date INT"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTablePlaylists:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    // Default Playlist: To Watch
    query.prepare("INSERT INTO playlists "
                  "VALUES(1, 'To Watch', 0, 0)");

    if (!query.exec()) {
        Macaw::DEBUG("In createTablePlaylists:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create the table `movies_playlists` which links movies to playlists
 * @param query
 * @return
 */
bool DatabaseManager::createTableMoviesPlaylists(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS movies_playlists("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "id_movie INTEGER NOT NULL, "
                  "id_playlist INTEGER NOT NULL, "
                  "UNIQUE (id_playlist, id_movie) ON CONFLICT IGNORE, "
                  "FOREIGN KEY(id_movie) REFERENCES movies ON DELETE CASCADE, "
                  "FOREIGN KEY(id_playlist) REFERENCES playlists ON DELETE CASCADE"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableMoviesPlaylists:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create the table `tags`, to store the tags that can be attributed to the movies
 * @param query
 * @return
 */
bool DatabaseManager::createTableTags(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS tags("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "name VARCHAR(255) UNIQUE NOT NULL"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableTags:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create the table `movies_tags` where are the links between tags and movies
 * @param query
 * @return
 */
bool DatabaseManager::createTableMoviesTags(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS movies_tags("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "id_movie INTEGER NOT NULL, "
                  "id_tag INTEGER NOT NULL, "
                  "UNIQUE (id_tag, id_movie) ON CONFLICT IGNORE, "
                  "FOREIGN KEY(id_movie) REFERENCES movies ON DELETE CASCADE, "
                  "FOREIGN KEY(id_tag) REFERENCES tags ON DELETE CASCADE"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableMoviesTags:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

bool DatabaseManager::createTableShow(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS show("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "name VARCHAR(255), "
                  "finished BOOLEAN"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableShow:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

bool DatabaseManager::createTableEpisodes(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS episodes("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "number INTEGER, "
                  "season INTEGER, "
                  "id_show INTEGER NOT NULL, "
                  "id_movie INTEGER UNIQUE NOT NULL, "
                  "FOREIGN KEY(id_movie) REFERENCES movies ON DELETE CASCADE, "
                  "FOREIGN KEY(id_show) REFERENCES show ON DELETE CASCADE "
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableEpisodes:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create table `path_list`, where the paths to import are stored
 * @param query
 * @return
 */
bool DatabaseManager::createTablePathList(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS path_list("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "movies_path VARCHAR(255) UNIQUE, "
                  "type INTEGER, "
                  "imported BOOLEAN DEFAULT 0"
                  ")");

    if (!query.exec()) {
        Macaw::DEBUG("In createTablePathList:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Create table `config`, for update purpose and app configuration
 * Then set the version of the db
 * @param query
 * @return
 */
bool DatabaseManager::createTableConfig(QSqlQuery &query)
{
    query.prepare("CREATE TABLE IF NOT EXISTS config("
                  "db_version INTEGER,"
                  "media_player VARCHAR(255))");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableConfig:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    // Set the database version
    query.prepare("INSERT INTO config (`db_version`) "
                  "VALUES ('" + QString::number(DB_VERSION) + "')");

    if (!query.exec()) {
        Macaw::DEBUG("In createTableConfig:");
        Macaw::DEBUG(query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief add a new tag with specified name to the database.
 * Returns the id of created tag, -1 if an error occurred.
 * @param name new tag's name
 * @returns the id of newly created tag in the db, -1 if an error occurs
 */
int DatabaseManager::createTag(QString name)
{
    QSqlQuery l_query(m_db);
    l_query.prepare("INSERT INTO `tags` (name) VALUES (:name)");
    l_query.bindValue(":name", name);

    if (!l_query.exec())
    {
        Macaw::DEBUG("In createTag():");
        Macaw::DEBUG(l_query.lastError().text());

        return -1;
    }

    if(l_query.exec("SELECT last_insert_rowid()"))
    {
        l_query.next();
        return l_query.value(0).toInt();
    }

    return -1;
}

/**
 * @brief Adds a movies directory
 *
 * @param PathForMovies moviesPath: containing the path to the movies directory
 * @return true if the paths list have been updated correctly
 */
bool DatabaseManager::addMoviesPath(PathForMovies moviesPath)
{
    QSqlQuery l_query(m_db);
    l_query.prepare("INSERT INTO path_list (movies_path, type) VALUES (:movies_path, :type)");
    l_query.bindValue(":movies_path", moviesPath.path());
    l_query.bindValue(":type", moviesPath.type());

    if(!l_query.exec())
    {
        Macaw::DEBUG("[DatabaseManager] In addMoviesPath():");
        Macaw::DEBUG(l_query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Adds a path to the media player
 *
 * @param QString mediaPlayerPath: containing the path to the media player
 * @return true if the path has been updated correctly
 */
bool DatabaseManager::addMediaPlayerPath(QString mediaPlayerPath)
{
    QSqlQuery l_query(m_db);
    l_query.prepare("DELETE FROM media_player");

    if(!l_query.exec()) {
        Macaw::DEBUG("[DatabaseManager] In addMediaPlayerPath():");
        Macaw::DEBUG(l_query.lastError().text());

        return false;
    }

    if (!mediaPlayerPath.trimmed().isEmpty()) {
        l_query.prepare("INSERT INTO media_player (media_player_path) VALUES (:media_player_path)");
        l_query.bindValue(":media_player_path", mediaPlayerPath);

        if(!l_query.exec()) {
            Macaw::DEBUG("[DatabaseManager] In addMediaPlayerPath():");
            Macaw::DEBUG(l_query.lastError().text());

            return false;
        }
    }

    return true;
}

/**
 * @brief Set the imported boolean of the movie path
 *
 * @param QString moviesPath: containing the path to the movies directory
 * @param bool imported: true if the movies have been imported, false else
 * @return true if the request succeed
 */
bool DatabaseManager::setMoviesPathImported(QString moviesPath, bool imported)
{
    QSqlQuery l_query(m_db);
    l_query.prepare("UPDATE path_list SET imported=:imported WHERE movies_path = :movies_path");
    l_query.bindValue(":movies_path", moviesPath);
    l_query.bindValue(":imported", imported);

    if(!l_query.exec())
    {
        Macaw::DEBUG("In setMoviesPathImported():");
        Macaw::DEBUG(l_query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Update a movies directory
 *
 * @param PathForMovies moviesPath: containing the path to the movies directory
 * @return true if the paths list have been updated correctly
 */
bool DatabaseManager::updateMoviesPath(PathForMovies moviesPath)
{
    QSqlQuery l_query(m_db);
    l_query.prepare("UPDATE path_list "
                    "SET movies_path=:movies_path, type=:type "
                    "WHERE id=:id");
    l_query.bindValue(":movies_path", moviesPath.path());
    l_query.bindValue(":type", moviesPath.type());
    l_query.bindValue(":id", moviesPath.id());

    if(!l_query.exec())
    {
        Macaw::DEBUG("[DatabaseManager] In updateMoviesPath():");
        Macaw::DEBUG(l_query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Get the movies directory having the id `id`
 * @param id
 * @return QString containing the path of this directory
 */
QString DatabaseManager::getMoviesPathById(int id)
{
    QString l_moviesPath;
    QSqlQuery l_query(m_db);
    l_query.prepare("SELECT movies_path FROM path_list WHERE id=:id");
    l_query.bindValue(":id", id);

    if(!l_query.exec())
    {
        Macaw::DEBUG("In getMoviesPathById():");
        Macaw::DEBUG(l_query.lastError().text());
    }

    if(l_query.next())
    {
        l_moviesPath = l_query.value(0).toString();
    }

    return l_moviesPath;
}

/**
 * @brief Get the movies directories
 * @param imported boolean telling if the movies of this path have already been imported
 * @return QStringList containing the paths of these directories
 */
QList<PathForMovies> DatabaseManager::getMoviesPaths(bool imported)
{
    QList<PathForMovies> l_moviesPathList;

    QSqlQuery l_query(m_db);
    l_query.prepare("SELECT id, movies_path, type FROM path_list where imported=:imported");
    l_query.bindValue(":imported", imported);

    if(!l_query.exec())
    {
        Macaw::DEBUG("In getMoviesPaths():");
        Macaw::DEBUG(l_query.lastError().text());
    }

    while(l_query.next())
    {
        PathForMovies l_moviesPath;
        l_moviesPath.setId(l_query.value(0).toInt());
        l_moviesPath.setPath(l_query.value(1).toString());
        l_moviesPath.setType(l_query.value(2).toInt());
        l_moviesPathList.append(l_moviesPath);
    }

    return l_moviesPathList;
}

bool DatabaseManager::existMoviesPath(PathForMovies moviesPath)
{
    return !this->getMoviesPathById(moviesPath.id()).isEmpty();
}

bool DatabaseManager::deleteMoviesPath(PathForMovies moviesPath)
{
    QList<Movie> l_movieList = getMoviesByPath(moviesPath);

    foreach (Movie l_movie, l_movieList) {
        if (!deleteMovie(l_movie))
        {

            return false;
        }
    }

    QSqlQuery l_query(m_db);
    l_query.prepare("DELETE FROM path_list WHERE movies_path LIKE :movies_path||'%'");
    l_query.bindValue(":movies_path", moviesPath.path());
    if(!l_query.exec())
    {
        Macaw::DEBUG("In removeMoviesPath(), deleting path:");
        Macaw::DEBUG(l_query.lastError().text());

        return false;
    }

    return true;
}

/**
 * @brief Get path to media player
 * @return QStringList containing the path to the media player
 */
QString DatabaseManager::getMediaPlayerPath()
{
    QSqlQuery l_query(m_db);
    l_query.prepare("SELECT media_player FROM config");

    if(!l_query.exec())
    {
        Macaw::DEBUG("In getMediaPlayerPath():");
        Macaw::DEBUG(l_query.lastError().text());
    }

    QString l_mediaPlayerPath("");

    while(l_query.next())
    {
        l_mediaPlayerPath.append(l_query.value(0).toString());
    }

    return l_mediaPlayerPath;
}
