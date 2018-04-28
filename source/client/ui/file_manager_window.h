//
// PROJECT:         Aspia
// FILE:            client/ui/file_manager_window.h
// LICENSE:         GNU General Public License 3
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#ifndef _ASPIA_CLIENT__UI__FILE_MANAGER_WINDOW_H
#define _ASPIA_CLIENT__UI__FILE_MANAGER_WINDOW_H

#include "protocol/file_transfer_session.pb.h"
#include "protocol/address_book.pb.h"
#include "ui_file_manager_window.h"

namespace aspia {

class FileManagerWindow : public QWidget
{
    Q_OBJECT

public:
    FileManagerWindow(proto::address_book::Computer* computer, QWidget* parent = nullptr);
    ~FileManagerWindow() = default;

public slots:
    void refresh();

signals:
    void windowClose();
    void localRequest(FileRequest* request);
    void remoteRequest(FileRequest* request);

protected:
    // QWidget implementation.
    void closeEvent(QCloseEvent* event) override;

private slots:
    void removeItems(FilePanel* sender, QList<FileRemover::Item> items);
    void sendItems(FilePanel* sender, QList<FileTransfer::Item> items);
    void receiveItems(FilePanel* sender, const QList<FileTransfer::Item>& items);

private:
    void transferItems(FileTransfer::Type type,
                       const QString& source_path,
                       const QString& target_path,
                       const QList<FileTransfer::Item>& items);

    Ui::FileManagerWindow ui;

    Q_DISABLE_COPY(FileManagerWindow)
};

} // namespace aspia

#endif // _ASPIA_CLIENT__UI__FILE_MANAGER_WINDOW_H
