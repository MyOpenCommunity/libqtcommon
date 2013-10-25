#ifndef ENTRYINFO_H
#define ENTRYINFO_H

#include <QHash>
#include <QString>
#include <QMetaType>
#include <QStringList>


/*!
	\ingroup Multimedia
	\brief Contains information about a resource entry.

	This structure contains the name, the type, the path (witch can be a path on
	the filesystem or an url), and optional metadata about a generic entry on
	filesystem or filesystem-structured resources (like UPnP).

	\sa TreeBrowser
*/
struct EntryInfo
{
	typedef QHash<QString,QString> Metadata;

	/*!
		\brief EntryInfo types.
	*/
	enum Type
	{
		UNKNOWN   = 0x01, /*!< Unknown filetype */
		DIRECTORY = 0x02, /*!< Directory */
		AUDIO     = 0x04, /*!< Audio filetype */
		VIDEO     = 0x08, /*!< Video filetype */
		IMAGE     = 0x10, /*!< Image filetype */
		ALL       = UNKNOWN | DIRECTORY | AUDIO | VIDEO | IMAGE /*!< All content */
	};

	/*!
		\brief The name of the entry.
		\note Mandatory.
	*/
	QString name;

	/*!
		\brief The type of the entry.
		\note Mandatory.
	*/
	EntryInfo::Type type;

	/*!
		\brief The path of the entry.
		\note Mandatory.
	*/
	QString path;

	/*!
		\brief The metadata of the entry.
		\note Optional.
	*/
	EntryInfo::Metadata metadata;

	/*!
		\brief Constructor.

		Constructs a new EntryInfo with the given \a name,\a type and path.
		Optionally you can pass the \a metadata associated with the entry.
	*/
	EntryInfo(const QString &name, EntryInfo::Type type, const QString &path, const EntryInfo::Metadata &metadata = EntryInfo::Metadata());
	EntryInfo();

	bool isNull();

private:
	bool is_null;
};

/*!
	\brief Returns true if \a a and \b are equal, false otherwise.
*/
inline bool operator ==(const EntryInfo &a, const EntryInfo &b)
{
	return a.name == b.name && a.type == b.type && a.path == b.path && a.metadata == b.metadata;
}

/*!
	List of EntryInfo.
*/
typedef QList<EntryInfo> EntryInfoList;

Q_DECLARE_METATYPE(EntryInfo);
Q_DECLARE_METATYPE(EntryInfoList);



/*!
	\ingroup Core
	\brief Returns a list of the recognized file types associated to \a type.
*/
QStringList getFileExtensions(EntryInfo::Type type);

/*!
	\ingroup Core
	\brief Returns a list of file filter expressions associated to \a type.
*/
QStringList getFileFilter(EntryInfo::Type type);

#endif // ENTRYINFO_H
