/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Copyable tag class
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_COPYABLE_HPP
#define TESLA_BASE_COPYABLE_HPP

namespace tesla
{

namespace base
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class Copyable
{
};

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_COPYABLE_HPP
