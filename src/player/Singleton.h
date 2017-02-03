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

#ifndef __TEMXCALL_SINGLETON_H__
#define __TEMXCALL_SINGLETON_H__

#include<stdio.h>
/**
 *******************************************************************************
 * @brief Singleton class template
 * Use this to make a class into a Singleton.
 * Example:
 * Class C { }; // C can be any class
 * typedef Singleton<C> C_Singleton; //C_Singleton is a Singleton of C
 *
 *******************************************************************************
 */

template <class T>
class Singleton
{
	public:
		/**
		*******************************************************************************
		* @brief Get the singleton instance.
		*
		* @retval - Returns pointer to the singleton instance
		*******************************************************************************
		*/
		static T& GetInstance()
		{
		   static T instance;

			return instance;
		}

	private:
		Singleton();
		~Singleton();
};


#endif /* __TEMXCALL_SINGLETON_H__ */
