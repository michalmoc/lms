/*
 * Copyright (C) 2018 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <map>
#include <set>

#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

namespace MetaData
{

	enum class Type
	{
		// Name			Type of the value
		Artist,			// string
		Title,			// string
		Album,			// string
		Clusters,		// Clusters, ex: { "genre", {"death metal", "brutal death"} }, { "albumgrouping", {"metal"} }
		Duration,		// std::chrono::milliseconds
		TrackNumber,		// size_t
		DiscNumber,		// size_t
		TotalTrack,		// size_t
		TotalDisc,		// size_t
		Year,			// int
		OriginalYear,		// int
		HasCover,		// bool
		AudioStreams,		// vector<AudioStream>
		MusicBrainzArtistID,	// string
		MusicBrainzAlbumID,	// string
		MusicBrainzTrackID,	// string
		MusicBrainzRecordingID,	// string
		AcoustID,		// string
	};

	// Used by Streams
	struct AudioStream
	{
		// TODO codec?
		std::size_t bitRate;
	};

	// Type and associated data
	// See enum Type's comments
	using Items = std::map<Type, boost::any>;
	using Clusters = std::map<std::string /* type */, std::set<std::string> /* names */>;
	using ClusterTypes = std::set<std::string>;

	class Parser
	{
		public:
			static const ClusterTypes defaultClusterTypes;

			// Provide a map for tag name -> Cluster name
			Parser(const ClusterTypes& clusterTypes) : _clusterTypes(clusterTypes) {}

			virtual boost::optional<Items> parse(const boost::filesystem::path& p, bool debug = false) = 0;

			void updateClusterTypes(const ClusterTypes& clusterTypes) { _clusterTypes = clusterTypes; }
			const ClusterTypes& getClusterTypes() const { return _clusterTypes; }
			bool isClusterTypeSupported(const std::string& clusterType) const { return _clusterTypes.find(clusterType) != _clusterTypes.end(); }

		protected:
			ClusterTypes _clusterTypes;
	};

} // namespace MetaData

