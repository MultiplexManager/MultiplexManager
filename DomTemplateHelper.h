#ifndef DOM_TEMPLATE_HELPER_H
#define DOM_TEMPLATE_HELPER_H

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QVariant>

template <typename T> void AddElement( QDomDocument* doc, QDomNode* parent, const char* name, T* field )
{
    QVariant var = *field;
    
    QDomElement element = doc->createElement( name );
    QDomText value = doc->createTextNode( var.toString() );

    element.appendChild( value );
    parent->appendChild( element );

}

template <typename T> void ReadElement( QDomNode* node, const char* name, T* field )
{
 
    const QDomElement & element = node->toElement();
    if ( element.isNull() )
    {
        return;
    }

    const QDomElement & elementWeWant = element.firstChildElement( name );
    if ( !elementWeWant.isNull() )
    {
        QDomCharacterData textNode =  elementWeWant.firstChild().toCharacterData();
        QVariant var( textNode.data() );
        if ( var.canConvert<T>() )
        {
            *field = var.value<T>();
        }
    }
}
#endif
