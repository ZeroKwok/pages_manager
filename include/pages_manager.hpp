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

#ifndef pages_manager_h__
#define pages_manager_h__

#include <QSet>
#include <QMap>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QStackedWidget>
#include <QApplication>

//! @brief 短码分配器(单例)
//! @note 为页面名称分配全局唯一的两字符短码
class ShortcodeAllocator
{
public:
    static ShortcodeAllocator& instance() {
        static ShortcodeAllocator* __imp = nullptr;
        if (__imp == nullptr)
            __imp = new ShortcodeAllocator();
        return *__imp;
    }

    //! @brief 为页面名称分配或获取短码
    //! @param pageName 页面名称
    //! @return 分配的短码 (2字符)
    QString allocate(const QString& pageName) {
        QString pageName_lower = pageName.toLower();
        
        // 如果已经分配过, 直接返回
        if (m_nameToCode.contains(pageName_lower))
            return m_nameToCode[pageName_lower];
        
        // 生成短码: 首字母 + 末字母
        QString code = generateBaseCode(pageName_lower);
        
        // 处理重复: 如果短码已被使用, 尝试生成新的
        int attempt = 0;
        while (m_codeToName.contains(code) && attempt < 26) {
            code = generateAlternativeCode(pageName_lower, attempt++);
        }
        
        Q_ASSERT_X(!m_codeToName.contains(code), "ShortcodeAllocator", 
                   QString("Short code collision: %1").arg(code).toStdString().c_str());
        
        m_nameToCode[pageName_lower] = code;
        m_codeToName[code] = pageName_lower;
        
        return code;
    }

    //! @brief 根据名称获取页面短码
    //! @param pageName 页面名称
    //! @return 页面短码, 不存在时返回空字符串
    QString pageCode(const QString& pageName) const {
        QString pageName_lower = pageName.toLower();
        if (m_nameToCode.contains(pageName_lower))
            return m_nameToCode[pageName_lower];
        return {};
    }

    //! @brief 根据短码获取页面名称
    //! @param code 短码
    //! @return 页面名称, 不存在时返回空字符串
    QString pageName(const QString& code) const {
        QString codeLower = code.toLower();
        if (m_codeToName.contains(codeLower))
            return m_codeToName[codeLower];
        return {};
    }

    //! @brief 清空所有分配记录
    void clear() {
        m_nameToCode.clear();
        m_codeToName.clear();
    }

    //! @brief 强制分配或验证短码
    //! @param pageName 页面名称
    //! @param customCode 指定的短码(可选), 如果为空则自动分配
    //! @return 分配的短码, 冲突返回空字符串
    QString assignShortcode(const QString& pageName, const QString& customCode = {}) {
        QString pageName_lower = pageName.toLower();
        
        // 如果已经分配过, 直接返回
        if (m_nameToCode.contains(pageName_lower))
            return m_nameToCode[pageName_lower];
        
        QString code = customCode.isEmpty() ? QString() : customCode.toLower();
        
        // 如果指定了短码，验证冲突
        if (!code.isEmpty()) {
            if (m_codeToName.contains(code))
                return {};  // 冲突
        }
        else {
            // 自动分配
            code = allocate(pageName);
            return code;
        }
        
        // 使用指定的短码
        m_nameToCode[pageName_lower] = code;
        m_codeToName[code] = pageName_lower;
        
        return code;
    }

private:
    ShortcodeAllocator() {}

    //! @brief 生成基础短码: 首字母 + 末字母
    QString generateBaseCode(const QString& name) const {
        if (name.length() < 2)
            return name.length() == 1 ? name + name : "";
        return QString(name[0]) + QString(name[name.length() - 1]);
    }

    //! @brief 生成替代短码 (冲突时使用)
    //! @note 首先尝试名字中的字符组合, 不足时使用任意字符尝试
    QString generateAlternativeCode(const QString& name, int attempt) const {
        if (name.isEmpty())
            return {};
        
        // 第一阶段: 尝试 name[0] + name[i] 的组合
        int nameLen = name.length();
        int nameAttempts = nameLen > 1 ? nameLen - 1 : nameLen;
        
        if (attempt < nameAttempts) {
            return QString(name[0]) + QString(name[attempt + 1]);
        }
        
        // 第二阶段: 名字组合都用完了, 使用任意字符尝试
        // 使用 a-z 字符的所有两字符组合
        int adjustedAttempt = attempt - nameAttempts;
        const QString chars = "abcdefghijklmnopqrstuvwxyz";
        int charsCount = chars.length();
        
        int pos1 = (adjustedAttempt / charsCount) % charsCount;
        int pos2 = adjustedAttempt % charsCount;
        
        return QString(chars[pos1]) + QString(chars[pos2]);
    }

private:
    QMap<QString, QString> m_nameToCode;  //!< pageName -> shortcode
    QMap<QString, QString> m_codeToName;  //!< shortcode -> pageName
};

// 前置声明
class PagesManager;
class PagesContainer;

//! @brief 抽象页面
//! @note 所有需要被纳入管理的页面必须继承此类。
class AbstractPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)

    friend class PagesContainer;
    friend class PagesManager;
public:
    AbstractPage(PagesContainer* container = nullptr);
    virtual ~AbstractPage() {}

    //! @brief 返回页面名称
    //! @note 页面名称在同一层级中应该唯一, 且不能为空.
    QString name() const { return m_name; }

    //! @brief 返回页面的唯一短码
    QString code() const { return m_shortcode; }

    //! @brief 返回页面路径
    //! @note 大小写不敏感, 页面路径应该唯一, 且不能为空.
    QString pagePath() const {
        QStringList hops(name());
        auto p = this;
        while (p = p->parentPage())
            hops.push_front(p->name());
        return "/" + hops.join('/');
    }

    //! @brief 返回父容器
    //! @note  页面必须安装在一个页面容器中, 否则将无法被管理。
    PagesContainer* parent() const { return m_parent; }

    //! @brief 返回此页面中挂载的页面容器
    //! @note  一个页面中可以挂载多个页面容器, 可以理解为树的分支。
    QSet<PagesContainer*> containers() const { return m_containers; }

    QVariant&    lastParam(const QString& key) { return m_lastParams[key]; }
    QVariantMap& lastParams() { return m_lastParams; }

    //! @brief 页面惰性初始化事件
    //! @note  pageLazyInit() 事件中页面期望: 自己真正被访问前才被初始化。
    //! 会在pageEnter(), pageInvoke(), pageRaises()之前被调用, 并保证每个页面实例仅调用一次.
    virtual void pageLazyInit() {};

    //! @brief 页面提升事件 (需要时, 被PagesManager调用)
    //! @note  pageRaises() 事件中页面期望: 将自身提升到最上层, 确保自己被激活时, 界面上显示的是自己。
    virtual void pageRaises();

    //! @brief 页面显示事件 (页面切换时, 被PagesManager调用)
    //! @note  pageShow() 事件中页面期望: 将 m_lastParams 中存储的状态, 同步到界面上。
    //! @note  pageShow(), pageEnter() 两者之前的区别是:
    //!          1. pageShow() 会在页面切换过程中被调用, 以便页面有机会刷新自己.
    //!          2. pageEnter() 只有在目标页面被激活 且 params 参数有效, 才会被调用.
    virtual void pageShow() {};

    //! @brief 页面数据进入事件 (带参数跳转到目标页面时, 被PagesManager调用)
    //! @param lastPath 上一个页面的路径
    //! @param params 页面参数，这个参数将在 pageEnter() 返回之后存储到 m_lastParams。
    //! @note  pageEnter() 事件中页面期望: 在必要时预处理 params 中的数据。
    //! @note  pageShow(), pageEnter() 两者之前的区别是:
    //!          1. pageShow() 会在页面切换过程中被调用, 以便页面有机会刷新自己.
    //!          2. pageEnter() 只有在目标页面被激活 且 params 参数有效, 才会被调用.
    virtual void pageEnter(QString lastPath, QVariantMap& params) {};

    //! @brief 页面调用事件
    //! @param callerPath 调用者的路径
    //! @param params 页面参数
    //! @return 返回值
    //! @note pageInvoke() 事件中页面期望: 响应来自其他页面或外部的 **事件以及数据**。
    virtual QVariant pageInvoke(QString callerPath, const QVariantMap& params) {
        Q_ASSERT_X(0, "AbstractPage", "This method is not implemented yet");
        return {};
    };

    //! @brief 调整到 path 指定的页面
    //! @param path 目标页面路径
    //! @param params 页面参数
    //! @note 跳转后将产生历史记录, 即可以通过 pageBack() 返回到之前的页面。
    virtual void pageGoto(QString path, const QVariantMap& params);

    //! @brief 返回到后一个页面
    //! @param params 页面参数, 如果不为空, 目标页面将会触发 pageEnter() 事件。
    //! @note 返回后将产生历史记录, 即可以通过 pageForward() 返回到之前的页面。
    virtual void pageBack(const QVariantMap& params);

    //! @brief 返回到前一个页面
    //! @param params 页面参数, 如果不为空, 目标页面将会触发 pageEnter() 事件。
    //! @note 返回后将产生历史记录, 即可以通过 pageBack() 返回到之前的页面。
    virtual void pageForward(const QVariantMap& params);

    //! @brief 安装页面容器
    //! @param container 容器实例
    //! @note 页面只能安装在页面容器中, 因此如果需要多级页面的话, 就需要在页面中挂载存放子页面的页面容器。
    virtual void installContainer(PagesContainer* container);

    //! @brief 返回自此页面以下的所有子、孙页面。
    const QMap<QString, AbstractPage*> subpages() const;

    //! @brief 返回此页面中第一个匹配 name 的子页面。
    AbstractPage* subpage(QString name) const;

    //! @brief 返回父页面
    //! @note parent()->parentPage()
    AbstractPage* parentPage() const;

protected:
    QString m_name;                     //!< 页面名称
    QString m_shortcode;                //!< 页面短码
    QVariantMap m_lastParams;           //!< 最近的页面入参
    PagesContainer* m_parent;           //!< 父容器
    QSet<PagesContainer*> m_containers; //!< 已安装的容器实例
};

//! @brief 页面容器(通常在UI设计师中从QStackedWidget提升过来)
//! @note 页面容器可以理解为页面路径中的分隔符 (/), 页面必须安装在页面容器中, 最顶端的页面容器称为 root.
class PagesContainer : public QStackedWidget
{
    Q_OBJECT
    friend class AbstractPage;
    friend class PagesManager;
public:
    PagesContainer(QWidget* parent = nullptr) 
        : QStackedWidget(parent)
        , m_parentPage(nullptr)
    {}

    virtual ~PagesContainer() {}

    //! @brief 安装页面到页面容器中, 并指定页面的名称
    //! @param name 页面名称, 在同一层级中应该唯一, 且不能为空.
    //! @param page 页面实例
    virtual void installPage(QString name, AbstractPage* page) {
        installPageWithCode(name, {}, page);
    }

    //! @brief 安装页面到页面容器中, 并指定页面的名称和短码
    //! @param name 页面名称, 在同一层级中应该唯一, 且不能为空.
    //! @param shortcode 页面短码(2字符), 如果为空则自动分配
    //! @param page 页面实例
    //! @note 如果短码冲突，将以致命错误结束
    virtual void installPageWithCode(QString name, QString shortcode, AbstractPage* page) {
        name = name.toLower();
        Q_ASSERT(!m_names.contains(name));
        
        // 分配或验证短码
        ShortcodeAllocator& allocator = ShortcodeAllocator::instance();
        QString assignedCode = allocator.assignShortcode(name, shortcode);
        
        if (assignedCode.isEmpty()) {
            Q_ASSERT_X(false, "PagesContainer::installPageWithCode", 
                       QString("Shortcode collision: %1").arg(shortcode).toStdString().c_str());
            return;
        }
        
        m_names[name] = QStackedWidget::addWidget(page);
        page->m_name = name;
        page->m_shortcode = assignedCode;
        page->m_parent = this;
    }

    //! @brief 获取指定名称的页面实例
    //! @param name 页面名称
    //! @note 指定的页面不存在将返回空指针.
    AbstractPage* page(QString name) const {
        name = name.toLower();
        if (m_names.contains(name))
            return static_cast<AbstractPage*>(QStackedWidget::widget(m_names[name]));
        return {};
    }

    //! @brief 获取父页面实例
    //! @note 页面容器可能会挂载在其他页面中, 以此形成多级页面, 没有父页面将返回空指针, 此时表示页面容器是 root.
    AbstractPage* parentPage() const {
        return m_parentPage;
    }

    //! @brief 获取容器中所有页面实例, 不包括下层页面.
    const QMap<QString, AbstractPage*> pages() const {
        QMap<QString, AbstractPage*> result;
        for (auto it = m_names.constBegin(); it != m_names.constEnd(); ++it)
            result[it.key()] = static_cast<AbstractPage*>(QStackedWidget::widget(it.value()));
        return result;
    }

    //! @brief 设置当前页面, 即将该页面置顶(显示在最上层)
    //! @param name 页面名称
    void setCurrentPage(QString name) {
        name = name.toLower();
        Q_ASSERT(m_names.contains(name));
        QStackedWidget::setCurrentIndex(m_names[name]);
    }

protected:
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

//! @brief 页面管理器(单例)
class PagesManager : public QObject
{
    Q_OBJECT
protected:
    PagesManager(QObject* parent)
        : QObject(parent)
    {}

public:
    //! @brief 获取页面管理器实例
    //! @note 线程不安全
    static PagesManager& instance() {
        static PagesManager* __imp = nullptr;
        if (__imp == nullptr)
            __imp = new PagesManager(qApp);
        return *__imp;
    }

    //! @brief 设置根容器
    void setRootContainer(PagesContainer* container) {
        Q_ASSERT(container);
        m_root = container;
        m_root->m_parentPage = nullptr;
    }

    //! @brief 返回所有的顶级页面
    const QMap<QString, AbstractPage*> topPages() {
        Q_ASSERT(m_root);
        return m_root->pages();
    }

    //! @brief 返回当前的活动页面
    //! @note 当前的活动页面有切只有一个，没有安装任何页面时将返回空指针.
    AbstractPage* currentPage() const { return m_currentPage; }

    //! @brief 返回指定路径的页面实例
    //! @note 如果页面未经过初始化, 则将触发 pageLazyInit() 事件.
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

            if (page == nullptr)
                return nullptr;

            if (!page->property("initialized").toBool()) {
                page->pageLazyInit();
                page->setProperty("initialized", true);
            }
        }

        return page;
    }

    QSet<PagesContainer*> containers(QString path) const {
        Q_ASSERT(m_root);
        if (path == "/")
            return { m_root };
        if (auto p = page(path))
            return p->m_containers;
        return {};
    }

    QStack<QString>& stackBack() { return m_stackBack; }

    QStack<QString>& stackForward() { return m_stackForward; }

    bool canForward() const {
        return !m_stackForward.isEmpty();
    }

    bool canBack() const {
        return !m_stackBack.isEmpty();
    }

    //! @brief 页面切换
    //! @param callerPagePath 发起切换的页面路径, 如果为空, 则表示由外部触发
    //! @param calleePagePath 要切换到的页面路径, 大小写不敏感
    //! @param params 页面参数, 如果不为空, 目标页面将会触发 pageEnter() 事件。
    //! @note 这种方式产生的切换不会产生历史, 页面成功切换后将发射 currentPageChanged() 信号。
    void pageSwitch(QString callerPagePath, QString calleePagePath, const QVariantMap& params)
    {
        Q_ASSERT(m_root);
        Q_ASSERT(!calleePagePath.contains("\\"));

        auto callPageEnter = [](auto & page, auto const& path, auto params) {
            page->pageEnter(path, params);
            page->m_lastParams = params;
        };

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
                callPageEnter(page, callerPagePath, params.value(path).value<QVariantMap>());
            else if (!params.isEmpty() && hops.last() == n)
                callPageEnter(page, callerPagePath, params);

            page->pageShow();
            page->pageRaises();
        }

        m_currentPage = page;
        emit currentPageChanged(callerPagePath, calleePagePath);
    }

    //! @brief 页面跳转
    //! @param callerPagePath 发起跳转的页面路径, 如果为空, 则表示由外部触发
    //! @param calleePagePath 要跳转的页面路径, 大小写不敏感
    //! @param params 页面参数, 如果不为空, 目标页面将会触发 pageEnter() 事件。
    //! @note 这种方式产生的切换将产生历史, 这意味着可以通过 pageBack() 和 pageForward() 进行导航, 页面成功跳转后将发射 currentPageChanged() 信号。
    void pageGoto(QString callerPagePath, QString calleePagePath, const QVariantMap& params) {
        callerPagePath = callerPagePath.toLower();
        calleePagePath = calleePagePath.toLower();

        if (callerPagePath.size())
            m_stackBack.push(callerPagePath);
        m_stackForward.clear();
        pageSwitch(callerPagePath, calleePagePath, params);
    }

    //! @brief 返回到前一个页面
    //! @param callerPagePath 发起跳转的页面路径, 不能为空.
    void pageForward(QString callerPagePath, const QVariantMap& params) {
        Q_ASSERT(canForward());
        m_stackBack.push(callerPagePath);
        pageSwitch(callerPagePath.toLower(), m_stackForward.pop(), params);
    }

    //! @brief 返回到后一个页面
    //! @param callerPagePath 发起跳转的页面路径, 不能为空.
    void pageBack(QString callerPagePath, const QVariantMap& params) {
        Q_ASSERT(canBack());
        m_stackForward.push(callerPagePath);
        pageSwitch(callerPagePath.toLower(), m_stackBack.pop(), params);
    }

    //! @brief 页面调用方法
    //! @param callerPagePath 发起调用的页面路径, 如果为空, 则表示由外部触发。
    //! @param calleePagePath 要调用的页面路径, 大小写不敏感
    //! @param params 页面参数, 目标页面将会触发 pageInvoke() 事件。
    //! @return 调用结果, 由页面的 pageInvoke() 事件返回。
    QVariant pageInvoke(QString callerPagePath, QString calleePagePath, const QVariantMap& params) {
        return page(calleePagePath)->pageInvoke(callerPagePath.toLower(), params);
    }

    //! @brief 将正常页面路径转换为短码路径
    //! @param normalPath 正常页面路径, 格式: /page1/page2/page3
    //! @return 短码路径, 格式: p1p2p3 (每个页面的短码连接, 无分隔符)
    QString toShortcodePath(QString normalPath) const {
        if (normalPath.isEmpty())
            return {};
        
        // 移除首尾的 /
        normalPath = normalPath.toLower();
        if (normalPath.startsWith("/"))
            normalPath = normalPath.mid(1);
        if (normalPath.endsWith("/"))
            normalPath = normalPath.left(normalPath.length() - 1);
        
        QStringList hops = normalPath.split("/", Qt::SkipEmptyParts);
        QString result;
        
        for (const auto& hop : hops) {
            result += ShortcodeAllocator::instance().pageCode(hop);;
        }
        
        return result;
    }

    //! @brief 将短码路径转换为正常页面路径
    //! @param shortcodePath 短码路径, 格式: p1p2p3 (每个短码为2字符)
    //! @return 正常页面路径, 格式: /page1/page2/page3, 转换失败返回空字符串
    QString fromShortcodePath(const QString& shortcodePath) const {
        if (shortcodePath.isEmpty() || shortcodePath.length() % 2 != 0)
            return {};
        
        QStringList hops;
        ShortcodeAllocator& allocator = ShortcodeAllocator::instance();
        
        // 按2字符一组分割并转换
        for (int i = 0; i < shortcodePath.length(); i += 2) {
            QString code = shortcodePath.mid(i, 2);
            QString name = allocator.pageName(code);
            
            if (name.isEmpty())
                return {};  // 短码不存在
            
            hops.append(name);
        }
        
        return "/" + hops.join("/");
    }

public:
    //! @brief 当前以任何方式切换当前页面时, 将发射此信号。
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
