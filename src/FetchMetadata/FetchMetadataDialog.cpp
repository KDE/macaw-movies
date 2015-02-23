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

#include "FetchMetadata/FetchMetadataDialog.h"
#include "ui_FetchMetadataDialog.h"

FetchMetadataDialog::FetchMetadataDialog(Movie movie, QList<Movie> moviesPropositionList, QWidget *parent):
    QDialog(parent),
    m_ui(new Ui::FetchMetadataDialog)
{
    m_app = qobject_cast<Application *>(qApp);
    m_app->debug("[FetchMetadataDialog] Constructor called");
    m_ui->setupUi(this);

    m_ui->lineEdit->setText(movie.title());
    m_ui->moviePathLabel->setText(movie.filePath());
    foreach (Movie l_movieProposition, moviesPropositionList)
    {
        QListWidgetItem *l_item = new QListWidgetItem;
        QString l_textItem = l_movieProposition.title()
                + " [" + QString::number(l_movieProposition.releaseDate().year())
                + "]";
        l_item->setText(l_textItem);
        l_item->setData(Macaw::ObjectId, l_movieProposition.id());
        m_ui->listWidget->addItem(l_item);
    }
}

FetchMetadataDialog::~FetchMetadataDialog()
{
    delete m_ui;
}


void FetchMetadataDialog::on_buttonBox_accepted()
{
    if (m_ui->listWidget->selectedItems().count() == 0)
    {
        QMessageBox::warning(this, "Warning", "You must select a movie!");
    }
    else
    {
        int tmdbID = m_ui->listWidget->selectedItems().at(0)->data(Macaw::ObjectId).toInt();
        emit(selectedMovie(tmdbID));
        this->accept();
    }
}

void FetchMetadataDialog::on_listWidget_doubleClicked(const QModelIndex &index)
{
    int tmdbID = index.data(Macaw::ObjectId).toInt();
    emit(selectedMovie(tmdbID));
    this->accept();
}
