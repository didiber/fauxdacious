/*
 * queue-manager.cc
 * Copyright 2014 William Pitcock, John Lindgren
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

#include "libfauxdqt.h"

#include <QAbstractListModel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPointer>

#include <libfauxdcore/audstrings.h>
#include <libfauxdcore/playlist.h>
#include <libfauxdcore/hook.h>
#include <libfauxdcore/i18n.h>

/*
 * TODO:
 *     - shifting of selection entries
 */

namespace audqt {

class QueueManagerModel : public QAbstractListModel
{
public:
    void update (QItemSelectionModel * sel);
    void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);

protected:
    int rowCount(const QModelIndex & parent) const
    {
        return parent.isValid () ? 0 : m_rows;
    }

    int columnCount (const QModelIndex & parent) const { return 2; }
    QVariant data (const QModelIndex & index, int role) const;

private:
    int m_rows = 0;
    bool m_in_update = false;
};

QVariant QueueManagerModel::data (const QModelIndex & index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int list = aud_playlist_get_active ();
        int entry = aud_playlist_queue_get_entry (list, index.row ());

        if (index.column () == 0)
            return entry + 1;
        else
        {
            Tuple tuple = aud_playlist_entry_get_tuple (list, entry, Playlist::NoWait);
            return QString ((const char *) tuple.get_str (Tuple::FormattedTitle));
        }
    }
    else if (role == Qt::TextAlignmentRole && index.column () == 0)
        return Qt::AlignRight;

    return QVariant ();
}

void QueueManagerModel::update (QItemSelectionModel * sel)
{
    int list = aud_playlist_get_active ();
    int rows = aud_playlist_queue_count (list);
    int keep = aud::min (rows, m_rows);

    m_in_update = true;

    if (rows < m_rows)
    {
        beginRemoveRows (QModelIndex (), rows, m_rows - 1);
        m_rows = rows;
        endRemoveRows ();
    }
    else if (rows > m_rows)
    {
        beginInsertRows (QModelIndex (), m_rows, rows - 1);
        m_rows = rows;
        endInsertRows ();
    }

    if (keep > 0)
    {
        auto topLeft = createIndex (0, 0);
        auto bottomRight = createIndex (keep - 1, 0);
        emit dataChanged (topLeft, bottomRight);
    }

    for (int i = 0; i < rows; i ++)
    {
        if (aud_playlist_entry_get_selected (list, aud_playlist_queue_get_entry (list, i)))
            sel->select (createIndex (i, 0), sel->Select | sel->Rows);
        else
            sel->select (createIndex (i, 0), sel->Deselect | sel->Rows);
    }

    m_in_update = false;
}

void QueueManagerModel::selectionChanged (const QItemSelection & selected,
 const QItemSelection & deselected)
{
    if (m_in_update)
        return;

    int list = aud_playlist_get_active ();

    for (auto & index : selected.indexes ())
        aud_playlist_entry_set_selected (list,
         aud_playlist_queue_get_entry (list, index.row ()), true);

    for (auto & index : deselected.indexes ())
        aud_playlist_entry_set_selected (list,
         aud_playlist_queue_get_entry (list, index.row ()), false);
}

class QueueManagerDialog : public QDialog
{
public:
    QueueManagerDialog (QWidget * parent = nullptr);

private:
    QTreeView m_treeview;
    QDialogButtonBox m_buttonbox;
    QPushButton m_btn_unqueue;
    QPushButton m_btn_close;
    QueueManagerModel m_model;

    void removeSelected ();
    void update () { m_model.update (m_treeview.selectionModel ()); }

    const HookReceiver<QueueManagerDialog>
     update_hook {"playlist update", this, & QueueManagerDialog::update},
     activate_hook {"playlist activate", this, & QueueManagerDialog::update};
};

QueueManagerDialog::QueueManagerDialog (QWidget * parent) :
    QDialog (parent)
{
    setWindowTitle (_("Queue Manager"));
    setContentsMargins (margins.TwoPt);

    m_btn_unqueue.setText (translate_str (N_("_Unqueue")));
    m_btn_close.setText (translate_str (N_("_Close")));

    connect (& m_btn_close, & QAbstractButton::clicked, this, & QWidget::hide);
    connect (& m_btn_unqueue, & QAbstractButton::clicked, this, & QueueManagerDialog::removeSelected);

    m_buttonbox.addButton (& m_btn_close, QDialogButtonBox::AcceptRole);
    m_buttonbox.addButton (& m_btn_unqueue, QDialogButtonBox::AcceptRole);

    auto layout = make_vbox (this);
    layout->addWidget (& m_treeview);
    layout->addWidget (& m_buttonbox);

    m_treeview.setAllColumnsShowFocus(true);
    m_treeview.setIndentation (0);
    m_treeview.setModel (& m_model);
    m_treeview.setSelectionMode (QAbstractItemView::ExtendedSelection);
    m_treeview.setHeaderHidden (true);

    update ();

    connect (m_treeview.selectionModel (),
     & QItemSelectionModel::selectionChanged, & m_model,
     & QueueManagerModel::selectionChanged);

    resize (4 * sizes.OneInch, 3 * sizes.OneInch);
}

void QueueManagerDialog::removeSelected ()
{
    int list = aud_playlist_get_active ();
    int count = aud_playlist_queue_count (list);

    for (int i = 0; i < count; )
    {
        int entry = aud_playlist_queue_get_entry (list, i);

        if (aud_playlist_entry_get_selected (list, entry))
        {
            aud_playlist_queue_delete (list, i, 1);
            aud_playlist_entry_set_selected (list, entry, false);
            count --;
        }
        else
            i ++;
    }
}

static QPointer<QueueManagerDialog> s_queuemgr;

EXPORT void queue_manager_show ()
{
    if (! s_queuemgr)
    {
        s_queuemgr = new QueueManagerDialog;
        s_queuemgr->setAttribute (Qt::WA_DeleteOnClose);
    }

    window_bring_to_front (s_queuemgr);
}

EXPORT void queue_manager_hide ()
{
    delete s_queuemgr;
}

} // namespace audqt
