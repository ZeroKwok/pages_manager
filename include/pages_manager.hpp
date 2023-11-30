// This file is part of the PagesManager distribution.
// Copyright (c) 2018-2023 zero.kwok@foxmail.com
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this software; 
// If not, see <http://www.gnu.org/licenses/>.

#ifndef pages_manager_h__
#define pages_manager_h__

#include <QSet>
#include <QMap>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QStackedWidget>
#include <QApplication>

//! 设计思路
//! 
//! 在一个UI具有多级页面的应用程序中, 内部的每一个UI页面需要互相解耦(即隔离--对彼此来说互相不可见),
//! 但又需要保证事件与数据在相互之间方便的传递, 还要支持多级页面的来回跳转。
//! 
//! 并且页面应该是无状态的, 由数据驱动(即由数据决定页面的表现, 且该数据不保存在页面内部)
//! 即:
//!     要实现一个浏览设备内容的 "浏览页面", 当浏览A设备时将产生一个 "A 设备内容" 的状态, 同理B设备也会产生一个 "B 设备内容" 的状态,
//!         分别记作: ViewPage(DeviceA), ViewPage(DeviceB).
//!     
//!     此时, 用户从 ViewPage(DeviceB) 页面跳转到 ViewPage(DeviceA), 之后点击 "back" 返回到 ViewPage(DeviceB), 
//!         再点击 "forward" 跳转 ViewPage(DeviceA). 在开发的角度上来说, 页面并没有切换, 但页面的内容不同了.
//! 
//!     通过通过无状态设计可以很好的实现这种需求, 可以将页面理解为一个视图, 视图要渲染的数据(如DeviceA的内容)则保存在某个地方, 
//!         我们只需要在页面切换时, 将数据传递进来由页面渲染即可.

//! 前置声明
class PagesManager;
class PagesContainer;

//!
//! 抽象页面
//! 
class AbstractPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)

    friend class PagesContainer;
    friend class PagesManager;
public:
    AbstractPage(PagesContainer* container = nullptr);
    virtual ~AbstractPage() {}

    QString name() const { return m_name; }

    //! 返回页面路径
    QString pagePath() const {
        QStringList hops(name());
        auto p = this;
        while (p = p->parentPage())
            hops.push_front(p->name());
        return "/" + hops.join('/');
    }

    //! 惰性初始化事件
    //! 会在pageEnter(), pageInvoke(), pageRaises()之前被调用, 并保证每个页面实例仅调用一次.
    virtual void pageLazyInit() {};
    virtual void pageRaises();

    //! pageShow(), pageEnter() 两者之前的区别是:
    //!   1. pageShow() 会在页面切换过程中被调用, 以便页面有机会渲染自己.
    //!   2. pageEnter() 只有在目标页面被激活 且 params参数有效, 才会被调用.

    virtual void pageShow() {};

    //! 注意:
    //! 参数 params 是非常量修饰, 而在pageEnter() 调用期间, m_lastParams仍然是上一次的状态.
    //! 这意味着你可以从 m_lastParams 拷贝一些信息保存在 params 里面.
    //! pageEnter()返回后, 页面管理器才将会将 params 覆盖到 m_lastParams.
    //! 
    virtual void pageEnter(QString lastPath, QVariantMap& params) {};
    virtual QVariant pageInvoke(QString callerPath, const QVariantMap& params) {
        Q_ASSERT_X(0, "AbstractPage", "This method is not implemented yet");
        return {};
    };

    virtual void pageGoto(QString path, const QVariantMap& params);
    virtual void pageBack(const QVariantMap& params);
    virtual void pageForward(const QVariantMap& params);

    virtual void installContainer(PagesContainer* container);

    const QMap<QString, AbstractPage*> subpages() const;

    AbstractPage* subpage(QString name) const;
    AbstractPage* parentPage() const;

protected:
    QString m_name;
    QVariantMap m_lastParams;           //!< 最近的页面入参
    PagesContainer* m_parent;           //!< 父容器
    QSet<PagesContainer*> m_containers; //!< 已安装的容器实例
};


//!
//! 页面容器(通常在ui设计师中从QStackedWidget提升过来)
//!
class PagesContainer : public QStackedWidget
{
    Q_OBJECT
    friend class AbstractPage;
    friend class PagesManager;
public:
    PagesContainer(QWidget* parent = nullptr) 
        : QStackedWidget(parent)
        , m_parentPage(nullptr)
    {
        //connect(this, &QStackedWidget::currentChanged, this, 
        //    [=](int index) {
        //        if (auto page = static_cast<AbstractPage*>(QStackedWidget::widget(index))) {
        //            if (!page->property("initialized").toBool()) {
        //                page->pageLazyInit();
        //                page->setProperty("initialized", true);
        //            }
        //        }
        //    });
    }

    virtual ~PagesContainer() {}

    virtual void installPage(QString name, AbstractPage* page) {
        name = name.toLower();
        Q_ASSERT(!m_names.contains(name));
        m_names[name] = QStackedWidget::addWidget(page);
        page->m_name = name;
        page->m_parent = this;
    }

    AbstractPage* page(QString name) const {
        name = name.toLower();
        if (m_names.contains(name))
            return static_cast<AbstractPage*>(QStackedWidget::widget(m_names[name]));
        return {};
    }

    AbstractPage* parentPage() const {
        return m_parentPage;
    }

    const QMap<QString, AbstractPage*> pages() const {
        QMap<QString, AbstractPage*> result;
        for (auto it = m_names.constBegin(); it != m_names.constEnd(); ++it)
            result[it.key()] = static_cast<AbstractPage*>(QStackedWidget::widget(it.value()));
        return result;
    }

    void setCurrentPage(QString name) {
        name = name.toLower();
        Q_ASSERT(m_names.contains(name));
        QStackedWidget::setCurrentIndex(m_names[name]);
    }

    void showEvent(QShowEvent* e) {
        if (auto page = static_cast<AbstractPage*>(QStackedWidget::widget(currentIndex()))) {
            if (!page->property("initialized").toBool()) {
                page->pageLazyInit();
                page->setProperty("initialized", true);
            }
        }
        QStackedWidget::showEvent(e);
    }

protected:
    QMap<QString, int> m_names;            //!< name -> QStackedWidget::index
    AbstractPage*      m_parentPage;       //!< 挂载容器的父页面, 只有root容器的父页面为nullptr
};

//!
//! 页面管理器(单例)
//!
class PagesManager : public QObject
{
    Q_OBJECT
protected:
    PagesManager(QObject* parent)
        : QObject(parent)
    {}

public:
    static PagesManager& instance() {
        static PagesManager* __imp = nullptr;
        if (__imp == nullptr)
            __imp = new PagesManager(qApp);
        return *__imp;
    }

    void setRootContainer(PagesContainer* container) {
        Q_ASSERT(container);
        m_root = container;
        m_root->m_parentPage = nullptr;
    }

    const QMap<QString, AbstractPage*> topPages() {
        Q_ASSERT(m_root);
        return m_root->pages();
    }

    AbstractPage* currentPage() const { return m_currentPage; }

    AbstractPage* page(QString path) const {
        Q_ASSERT(m_root);
        Q_ASSERT(!path.contains("\\"));
        path = path.toLower();

        QStringList hops = path.split("/", Qt::SkipEmptyParts);
        Q_ASSERT(!hops.isEmpty());

        AbstractPage* page = nullptr;
        for (auto n : hops)
        {
            if (page == nullptr)
                page = m_root->page(n);
            else
                page = page->subpage(n);

            Q_ASSERT(page);
            if (!page->property("initialized").toBool()) {
                page->pageLazyInit();
                page->setProperty("initialized", true);
            }
        }

        return page;
    }

    bool canForward() const {
        return !m_stackForward.isEmpty();
    }

    bool canBack() const {
        return !m_stackBack.isEmpty();
    }

    void pageSwitch(QString callerPagePath, QString calleePagePath, const QVariantMap& params)
    {
        Q_ASSERT(m_root);
        Q_ASSERT(!calleePagePath.contains("\\"));

        callerPagePath = callerPagePath.toLower();
        calleePagePath = calleePagePath.toLower();

        QStringList hops = calleePagePath.split("/", Qt::SkipEmptyParts);
        Q_ASSERT(!hops.isEmpty());

        AbstractPage* page = nullptr;
        for (auto n : hops)
        {
            if (page == nullptr)
                page = m_root->page(n);
            else
                page = page->subpage(n);

            Q_ASSERT(page);
            if (!page->property("initialized").toBool()) {
                page->pageLazyInit();
                page->setProperty("initialized", true);
            }

            auto path = page->pagePath();
            if (params.contains(path))
                page->pageEnter(callerPagePath, params.value(path).value<QVariantMap>());

            page->pageShow();
            page->pageRaises();
        }

        if (!params.isEmpty()) {
            auto duplicate = params;
            page->pageEnter(callerPagePath, duplicate);
            page->m_lastParams = duplicate;
        }

        m_currentPage = page;
        emit currentPageChanged(callerPagePath, calleePagePath);
    }

    void pageGoto(QString callerPagePath, QString calleePagePath, const QVariantMap& params) {
        callerPagePath = callerPagePath.toLower();
        calleePagePath = calleePagePath.toLower();

        if (callerPagePath.size())
            m_stackBack.push(callerPagePath);
        m_stackForward.clear();
        pageSwitch(callerPagePath, calleePagePath, params);
    }

    void pageForward(QString callerPagePath, const QVariantMap& params) {
        Q_ASSERT(canForward());
        m_stackBack.push(callerPagePath);
        pageSwitch(callerPagePath.toLower(), m_stackForward.pop(), params);
    }

    void pageBack(QString callerPagePath, const QVariantMap& params) {
        Q_ASSERT(canBack());
        m_stackForward.push(callerPagePath);
        pageSwitch(callerPagePath.toLower(), m_stackBack.pop(), params);
    }

    QVariant pageInvoke(QString callerPagePath, QString calleePagePath, const QVariantMap& params) {
        return page(calleePagePath)->pageInvoke(callerPagePath.toLower(), params);
    }

public:
    Q_SIGNAL void currentPageChanged(QString oldPagePath, QString newPagePath);

protected:
    PagesContainer* m_root = nullptr;
    AbstractPage*   m_currentPage = nullptr;
    QStack<QString> m_stackBack;
    QStack<QString> m_stackForward;
};

//////////////////////////////////////////////////////////////////////////

inline AbstractPage::AbstractPage(PagesContainer* container /*= nullptr*/)
    : QWidget(container)
    , m_parent(container)
{}

inline void AbstractPage::pageRaises() {
    Q_ASSERT(m_parent);
    m_parent->setCurrentPage(name());
}

inline const QMap<QString, AbstractPage*> AbstractPage::subpages() const {
    if (m_containers.isEmpty())
        return {};
    QMap<QString, AbstractPage*> result;
    for (auto c : m_containers)
        result.insert(c->pages());
    return result;
}

inline AbstractPage* AbstractPage::subpage(QString name) const {
    name = name.toLower();
    for (auto c : m_containers)
        if (auto p = c->page(name))
            return p;
    return {};
}

inline AbstractPage* AbstractPage::parentPage() const {
    if (m_parent)
        return m_parent->parentPage();
    return {};
}

inline void AbstractPage::pageGoto(QString path, const QVariantMap& params) {
    PagesManager::instance().pageGoto(pagePath(), path, params);
}

inline void AbstractPage::pageBack(const QVariantMap& params) {
    PagesManager::instance().pageBack(pagePath(), params);
}

inline void AbstractPage::pageForward(const QVariantMap& params) {
    PagesManager::instance().pageForward(pagePath(), params);
}

inline void AbstractPage::installContainer(PagesContainer* container) {
    Q_ASSERT(container);
    m_containers.insert(container);
    container->m_parentPage = this;
}

#endif // pages_manager_h__
