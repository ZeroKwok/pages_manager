// Copyright (c) 2022-2024 Zero <zero.kwok@foxmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pages_manager.hpp"
#include <QtWidgets>

class MyPage : public AbstractPage
{
    Q_OBJECT
public:
    MyPage()
        : AbstractPage()
    {
        _label = new QLabel();
        _content = new QWidget();

        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(_label);
        vbox->addWidget(_content);
        vbox->setStretchFactor(_content, 1);
        setLayout(vbox);
    }

    void pageLazyInit() override {
        _label->setText(pagePath());
    }

    void setupWidget(QWidget* widget) {
        QHBoxLayout* hbox = new QHBoxLayout(_content);
        hbox->addWidget(widget);
    }

    void pageEnter(QString lastPath, QVariantMap& params) {
        
    }

    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.setPen(generateColorFromString(name()));
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }

    QColor generateColorFromString(const QString& inputString) {
        QByteArray hash = QCryptographicHash::hash(inputString.toUtf8(), QCryptographicHash::Md5);
        quint8 r = (hash[0] + hash[1]) % 256;
        quint8 g = (hash[2] + hash[3]) % 256;
        quint8 b = (hash[4] + hash[5]) % 256;
        return QColor(r, g, b);
    }

protected:
    QLabel* _label;
    QWidget* _content;
};

QStandardItem* FeedTreeModel(QStandardItem* data, QMap<QString, AbstractPage*> pages)
{
    QList<QStandardItem*> items;
    for (auto it = pages.constBegin(); it != pages.constEnd(); ++it)
    {
        auto item = new QStandardItem(it.key());
        item->setData(it.value()->pagePath(), Qt::UserRole);
        items << item;
        FeedTreeModel(item, it.value()->subpages());
    }

    if (items.size())
        data->appendRows(items);

    return data;
};

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QDialog dialog;
    auto list = new QTreeView();
    auto data = new QStandardItemModel(&a);
    
    auto vbox = new QVBoxLayout();
    
    list->setModel(data);
    list->setHeaderHidden(true);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setMinimumWidth(200);
    vbox->addWidget(list);
    {
        auto _goto = new QPushButton("Goto");
        auto _back = new QPushButton("Back");
        auto _forward = new QPushButton("Forward");
        auto _edit = new QLineEdit();

        auto hbox = new QHBoxLayout;
        hbox->addWidget(_back);
        hbox->addWidget(_forward);
        hbox->addWidget(_goto);

        vbox->addLayout(hbox);
        vbox->addWidget(_edit);

        QObject::connect(_back, &QPushButton::clicked, &PagesManager::instance(), [=] {
            PagesManager::instance().pageBack(
                PagesManager::instance().currentPage()->pagePath(), {});
        });

        QObject::connect(_forward, &QPushButton::clicked, &PagesManager::instance(), [=] {
            PagesManager::instance().pageForward(
                PagesManager::instance().currentPage()->pagePath(), {});
            });

        QObject::connect(_goto, &QPushButton::clicked, &PagesManager::instance(), [=] {
            PagesManager::instance().pageGoto(
                PagesManager::instance().currentPage()->pagePath(), _edit->text(), {});
            });

        QObject::connect(&PagesManager::instance(), &PagesManager::currentPageChanged,
            &dialog, [=](QString oldPagePath, QString newPagePath) {
                _back->setEnabled(PagesManager::instance().canBack());
                _forward->setEnabled(PagesManager::instance().canForward());
            });
    }

    auto hbox = new QHBoxLayout();
    auto root = new PagesContainer();
    PagesManager::instance().setRootContainer(root);
    hbox->addLayout(vbox);
    hbox->addWidget(root);
    hbox->setStretchFactor(vbox, 1);
    hbox->setStretchFactor(root, 3);
    dialog.setLayout(hbox);

    root->installPage("home", new MyPage());
    root->installPage("view", new MyPage());
    root->installPage("perform", new MyPage());

    //////////////////////////////////////////////////////////////////////////
    auto pageCast = [](QString p) {
        return static_cast<MyPage*>(PagesManager::instance().page(p));
    };

    if (auto page = pageCast("/home"))
    {
        auto homeContainer = new PagesContainer();
        homeContainer->installPage("backup", new MyPage());
        homeContainer->installPage("history", new MyPage());
        homeContainer->installPage("tools", new MyPage());
        homeContainer->installPage("demo", new MyPage());
        page->setupWidget(homeContainer);
        page->installContainer(homeContainer);
    }

    if (auto page = pageCast("/view"))
    {
        auto viewContainer = new PagesContainer();
        viewContainer->installPage("photos", new MyPage());
        viewContainer->installPage("contacts", new MyPage());
        viewContainer->installPage("messages", new MyPage());
        page->setupWidget(viewContainer);
        page->installContainer(viewContainer);
    }

    if (auto page = pageCast("/perform"))
    {
        auto performContainer = new PagesContainer();
        performContainer->installPage("progress", new MyPage());
        performContainer->installPage("finished", new MyPage());
        performContainer->installPage("failed", new MyPage());
        page->setupWidget(performContainer);
        page->installContainer(performContainer);
    }

    if (auto page = pageCast("/home/demo"))
    {
        auto frame = new QWidget();
        auto hbox1 = new QHBoxLayout(frame);
        auto homeDemoContainer0 = new PagesContainer();
        auto homeDemoContainer1 = new PagesContainer();
        homeDemoContainer0->installPage("left1", new MyPage());
        homeDemoContainer0->installPage("left2", new MyPage());
        homeDemoContainer1->installPage("right1", new MyPage());
        homeDemoContainer1->installPage("right2", new MyPage());
        hbox1->addWidget(homeDemoContainer0);
        hbox1->addWidget(homeDemoContainer1);

        page->setupWidget(frame);
        page->installContainer(homeDemoContainer0);
        page->installContainer(homeDemoContainer1);
    }

    //////////////////////////////////////////////////////////////////////////

    PagesManager::instance().pageGoto({}, "/home", {});
    data->setHorizontalHeaderLabels(QStringList() << "path");
    data->appendRow(FeedTreeModel(
        new QStandardItem("root"), 
        PagesManager::instance().topPages()));
    list->expandAll();

    QObject::connect(list, &QAbstractItemView::doubleClicked, 
        &PagesManager::instance(), [=](QModelIndex index) {
            auto path = index.data(Qt::UserRole).toString();
            if (path.size())
                PagesManager::instance().pageGoto(
                    PagesManager::instance().currentPage()->pagePath(), path, {});
        });

    QObject::connect(&PagesManager::instance(), &PagesManager::currentPageChanged,
        list, [=](QString oldPagePath, QString newPagePath) {
            QModelIndexList indexes = data->match(data->index(0, 0, QModelIndex()),
                Qt::UserRole, newPagePath, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
            if (indexes.size())
                list->setCurrentIndex(indexes.first());
        });

    dialog.show();
    return a.exec();
}

#include "pages_manager.moc"

