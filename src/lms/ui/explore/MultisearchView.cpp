#include "MultisearchView.hpp"

#include <Wt/WButtonGroup.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WRadioButton.h>
#include <database/Artist.hpp>
#include <database/Release.hpp>

#include "ArtistListHelpers.hpp"
#include "core/ILogger.hpp"
#include "database/Session.hpp"
#include "database/Track.hpp"

#include "LmsApplication.hpp"
#include "MultisearchListHelpers.hpp"
#include "common/InfiniteScrollingContainer.hpp"
#include "explore/Filters.hpp"
#include "explore/TrackListHelpers.hpp"

namespace lms::ui
{
    using namespace db;

    Multisearch::Multisearch(Filters& filters, PlayQueueController& playQueueController, Wt::WLineEdit& searEdit)
        : Template{ Wt::WString::tr("Lms.Explore.Multisearch.template") }
        , _filters{ filters }
        , _playQueueController{ playQueueController }
        , _multisearchCollector{ filters, DatabaseCollectorBase::Mode::All, _maxCount }
    {
        addFunction("tr", &Wt::WTemplate::Functions::tr);
        addFunction("id", &Wt::WTemplate::Functions::id);

        searEdit.setPlaceholderText(Wt::WString::tr("Lms.Explore.Search.search-placeholder"));
        searEdit.textInput().connect([this, &searEdit] {
            if (wApp->internalPath() != "/multisearch")
            {
                wApp->setInternalPath("/multisearch", true);
            }
            refreshView(searEdit.text());
        });

        _mediaTypeFilters = std::make_shared<Wt::WButtonGroup>();
        _mediaTypeFilters->addButton(bindNew<Wt::WRadioButton>("search-all"), static_cast<int>(any_medium::Type::ALL));
        _mediaTypeFilters->addButton(bindNew<Wt::WRadioButton>("search-releases"), static_cast<int>(any_medium::Type::RELEASES));
        _mediaTypeFilters->addButton(bindNew<Wt::WRadioButton>("search-artists"), static_cast<int>(any_medium::Type::ARTISTS));
        _mediaTypeFilters->addButton(bindNew<Wt::WRadioButton>("search-tracks"), static_cast<int>(any_medium::Type::TRACKS));
        _mediaTypeFilters->setCheckedButton(_mediaTypeFilters->button(static_cast<int>(any_medium::Type::ALL)));
        _mediaTypeFilters->checkedChanged().connect([this]() { refreshView(); });

        _container = bindNew<InfiniteScrollingContainer>("multisearch-results", Wt::WString::tr("Lms.Explore.Multisearch.template.entry-container"));
        _container->onRequestElements.connect([this] {
            addSome();
        });

        filters.updated().connect([this] {
            refreshView();
        });

        refreshView();
    }

    void Multisearch::refreshView()
    {
        _container->reset();
    }

    void Multisearch::refreshView(const Wt::WString& searchText)
    {
        _multisearchCollector.setSearch(searchText.toUTF8());
        refreshView();
    }

    namespace
    {
        template<typename IdT>
        void findAndAdd(const IdT& mediumId, Filters& filters, PlayQueueController& playQueueController, InfiniteScrollingContainer& container)
        {
            if (const auto result = IdT::Target::find(LmsApp->getDbSession(), mediumId))
                container.add(MultisearchListHelpers::createEntry(result, playQueueController, filters));
        }
    } // namespace

    void Multisearch::addSome()
    {
        const auto [_, results, moreResults] = _multisearchCollector.get(static_cast<any_medium::Type>(_mediaTypeFilters->checkedId()), Range{ _container->getCount(), _batchSize });

        auto transaction = LmsApp->getDbSession().createReadTransaction();
        for (const auto mediumId : results)
        {
            std::visit([this](auto&& mediumId) { findAndAdd(mediumId, _filters, _playQueueController, *_container); }, mediumId);
        }

        _container->setHasMore(moreResults);
    }
} // namespace lms::ui