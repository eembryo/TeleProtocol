
/*
 * @@@ LICENSE
 * Copyright (C) 2015-2016, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 * Design/Author : dhilipkumar.raman@lge.com, jinheung.tark@lgepartner.com
 * Date : 02/04/2016
 *
 * <IHU XCallService> Detailed note is as follows.
 *
 *
 */
 
/**
 * ****************************************************************************
 * @file ObjectFactory.h
 *
 * Copyright (c) 2009 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * @brief  Implementation of Object Factory design pattern,
 *		 based on Factory.h in Andrei Alexandrescu's Loki
 *		 Library.  The Loki library is licensed under MIT
 *		 License -- see "The Loki Library" Licensing terms in
 *		 the header below.
 *
 * @author vitaly
 *
 * @version  0.01  2009-01-28  <vmk>  Initial creation; added GetIdentifiers
 * ****************************************************************************
 */

////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// Copyright (c) 2005 by Peter Kuemmel
// This code DOES NOT accompany the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//	 Patterns Applied". Copyright (c) 2001. Addison-Wesley.
//
// Code covered by the MIT License
// The authors make no representations about the suitability of this software
// for any purpose. It is provided "as is" without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H


/**
 * @brief		Implements a generic object factory.
 *
 * <i>The Factory Method pattern is an object-oriented design pattern.
 * Like other creational patterns, it deals with the problem of creating objects
 * (products) without specifying the exact class of object that will be created.
 * Factory Method, one of the patterns from the Design Patterns book, handles
 * this problem by defining a separate method for creating the objects, which
 * subclasses can then override to specify the derived type of product that will
 * be created.
 * <br>
 * More generally, the term Factory Method is often used to refer to any method
 * whose main purpose is creation of objects.</i>
 * <div ALIGN="RIGHT"><a href="http://en.wikipedia.org/wiki/Factory_method_pattern">
 * Wikipedia</a></div>
 *
 * Loki proposes a generic version of the Factory. Here is a typical use.<br>
 * <code><br>
 * 1. Factory< AbstractProduct, int > aFactory;<br>
 * 2. aFactory.Register( 1, createProductNull );<br>
 * 3. aFactory.CreateObject( 1 ); <br>
 * </code><br>
 * <br>
 * - 1. The declaration<br>
 * You want a Factory that produces AbstractProduct.<br>
 * The client will refer to a creation method through an int.<br>
 * - 2.The registration<br>
 * The code that will contribute to the Factory will now need to declare its
 * ProductCreator by registering them into the Factory.<br>
 * A ProductCreator is a just a function that will return the right object. ie <br>
 * <code>
 * Product* createProductNull()<br>
 * {<br>
 *	 return new Product<br>
 * }<br>
 * </code><br>
 * - 3. The use<br>
 * Now the client can create object by calling the Factory's CreateObject method
 * with the right identifier. If the ProductCreator were to have arguments
 * (<i>ie :Product* createProductParm( int a, int b )</i>)
 */

#include <map>
#include <exception>
#include <vector>
#include <tr1/functional>


/*
 * @brief
 * Define a null structure which is used as default for the param arguments of
 * the object factory template
 */
struct NullType{ };


/**
  * ************************************************************************
  * @brief class template DefaultFactoryError
  *
  * Handles the "Unknown Type" error in an object factory
  * ************************************************************************
  */
template <typename IdentifierType, class AbstractProduct>
struct DefaultFactoryError
{
	struct Exception : public std::exception
	{
		const char* what() const throw() { return "Unknown Type"; }
	};

	static AbstractProduct* OnUnknownType(IdentifierType)
	{
		throw Exception();
	}
};

/**
  * ************************************************************************
  * @brief class template ObjectFactory
  *
  * Implements a generic object factory which can take product creators with
  * upto 10 parameters. Since variadic types are not implemented in c++ yet,
  * and also because I want to keep the implementation of the object factory
  * simple, without writing my own implementation of variadic arguments, hence
  * this rudimentary Object factory template.
  * @see Loki Library for examples of how variadic arguments is implemented
  *
  * @param[AbstractProduct] This is the concrete product created and returned
  *						 from the factory
  * @param[IdentifierType]  This serves as an ID for the product to be created.
  *						 So registration happens with this ID type and the
  *						 method CreateProduct uses it as well.
  * @param[ProductCreator]  This is the creator function which is stored in the
  *						 factory with the ID.
  * @param [Param[1-10]]	Initialized with Null type. Allows the ProductCreator
  *						 with at most 10 arguments.
  * @param [FactoryErrorPolicy] Defines the actions when the factory does not
  *							 contain the required product
  *
  * @note This class is NOT thread-safe.  If thread-safety is
  *	   needed, it should be added externally, by wrapping this
  *	   class in a thread-safe API.
  *
  * ************************************************************************
  */

template
<
	class AbstractProduct,
	typename IdentifierType,
	typename ProductCreator,
	typename Param1 = NullType,
	typename Param2 = NullType,
	typename Param3 = NullType,
	typename Param4 = NullType,
	typename Param5 = NullType,
	typename Param6 = NullType,
	typename Param7 = NullType,
	typename Param8 = NullType,
	typename Param9 = NullType,
	typename Param10 = NullType,
	template<typename, class>
		class FactoryErrorPolicy = DefaultFactoryError
>
class ObjectFactory
	: public FactoryErrorPolicy<IdentifierType, AbstractProduct>
{
public:

	/**
	  * Registers a product creator
	  *
	  * @param id product class identifier
	  * @param creator product instance creator function/functor
	  *				(depending on definition of ProductCreator)
	  *
	  * @return bool
	  */
	bool Register(const IdentifierType& id, ProductCreator creator)
	{
	   //printf("******* Register %s \n", id.c_str());
		return associations_.insert(
			typename IdToProductMap::value_type(id, creator)).second != 0;
	}

	/**
	  * Unregisters a product instance, given its product class
	  * identifier
	  *
	  * @param id
	  *
	  * @return bool true on success; false if product class
	  *		 identifier not found
	  */
	bool Unregister(const IdentifierType& id)
	{
		return associations_.erase(id) != 0;
	}

	/**
	  * Creates a product instance, given its product class
	  * identifier
	  *
	  * @param id
	  *
	  * @return AbstractProduct*
	  */
	AbstractProduct* CreateObject(const IdentifierType& id)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)();
		}
		return this->OnUnknownType(id);
	}


	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2,
		Param3 p3, Param4 p4)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5, Param6 p6)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5, p6);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5, Param6 p6, Param7 p7)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5, p6, p7);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5, p6, p7, p8);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5, p6, p7, p8, p9);
		}
		return this->OnUnknownType(id);
	}

	AbstractProduct* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3,
		Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10)
	{
	   //printf("******* CreateObject %s \n", id.c_str());
		typename IdToProductMap::iterator i = associations_.find(id);
		if (i != associations_.end())
		{
			return (i->second)(p1,p2, p3, p4, p5, p6, p7, p8, p9, p10);
		}
		return this->OnUnknownType(id);
	}

	/**
	  * Returns a vector of registered product creator identifiers.
	  * Useful for diagnostics.
	  *
	  * @return std::vector<IdentifierType>
	  */
	std::vector<IdentifierType> GetRegisteredIds()
	{
		std::vector<IdentifierType> ids;
		for(typename IdToProductMap::iterator it = associations_.begin();
			it != associations_.end(); ++it)
		{
			ids.push_back(it->first);
		}
		return ids;
	}

private:
	typedef std::map<IdentifierType, ProductCreator> IdToProductMap;
	IdToProductMap associations_;
};

#endif // OBJECT_FACTORY_H
