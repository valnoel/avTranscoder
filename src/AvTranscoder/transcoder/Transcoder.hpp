#ifndef _AV_TRANSCODER_TRANSCODER_HPP_
#define _AV_TRANSCODER_TRANSCODER_HPP_

#include <AvTranscoder/common.hpp>

#include <AvTranscoder/file/InputFile.hpp>
#include <AvTranscoder/file/OutputFile.hpp>

#include <AvTranscoder/codedStream/IInputStream.hpp>

#include <AvTranscoder/essenceStream/GeneratorAudio.hpp>
#include <AvTranscoder/essenceStream/GeneratorVideo.hpp>

#include <AvTranscoder/ProfileLoader.hpp>

#include "StreamTranscoder.hpp"

#include <string>
#include <vector>

namespace avtranscoder
{

/**
 * @brief Enum to set a policy of how we manage the transcode in case of several streams.
 * eProcessMethodShortest: stop transcode at the end of the shortest stream.
 * eProcessMethodLongest: stop transcode at the end of the longest stream (default method).
 * eProcessMethodBasedOnStream: stop transcode at the end of an indicated stream (@see _indexBasedStream of Transcoder).
 * eProcessMethodInfinity: stop transcode by outside of avTranscoder.
 */
enum EProcessMethod
{
	eProcessMethodShortest = 0,
	eProcessMethodLongest,
	eProcessMethodBasedOnStream,
	eProcessMethodInfinity,
};

/**
 * @brief A Transcoder manages a list of streams, 
 * and process a transcode to create an output media file.
 */
class AvExport Transcoder
{
public:
	/**
	 * @note Set FFmpeg log level to quite.
	 */
	Transcoder( OutputFile& outputFile );
	
	~Transcoder();

	/**
	 * @brief Add a stream and set a profile
	 * @note If profileName is empty, rewrap.
	 */
	void add( const std::string& filename, const size_t streamIndex, const std::string& profileName = "", const size_t offset = 0 );
	/*
	 * @note If filename is empty, add a generated stream.
	 * @note If filename is empty, profileName can't be empty (no sens to rewrap a generated stream).
	 */
	void add( const std::string& filename, const size_t streamIndex, const std::string& profileName, ICodec& codec, const size_t offset = 0 );

	/**
	 * @brief Add a stream and set a custom profile
	 * @note Profile will be updated, be sure to pass unique profile name.
	 */
	void add( const std::string& filename, const size_t streamIndex, ProfileLoader::Profile& profile, const size_t offset = 0 );
	/*
	 * @note If filename is empty, add a generated stream.
	 */
	void add( const std::string& filename, const size_t streamIndex, ProfileLoader::Profile& profile, ICodec& codec, const size_t offset = 0  );
	
	/**
	 * @brief Add a stream and set a profile
	 * @note If profileName is empty, rewrap.
	 * @note If subStreamIndex is negative, no substream is selected it's the stream. 
	 */
	void add( const std::string& filename, const size_t streamIndex, const int subStreamIndex, const std::string& profileName = "", const size_t offset = 0 );
	/**
	 * @note If filename is empty, add a generated stream.
	 * @note If filename is empty, profileName can't be empty (no sens to rewrap a generated stream).
	 */
	void add( const std::string& filename, const size_t streamIndex, const int subStreamIndex, const std::string& profileName, ICodec& codec, const size_t offset = 0  );

	/**
	 * @brief Add a stream and set a custom profile
	 * @note Profile will be updated, be sure to pass unique profile name.
	 * @note If subStreamIndex is negative, no substream is selected it's the stream.
	 */
	void add( const std::string& filename, const size_t streamIndex, const int subStreamIndex, ProfileLoader::Profile& profile, const size_t offset = 0 );
	/**
	 * @note If filename is empty, add a generated stream.
	 */
	void add( const std::string& filename, const size_t streamIndex, const int subStreamIndex, ProfileLoader::Profile& profile, ICodec& codec, const size_t offset = 0  );

	/**
	 * @brief Add the stream
	 */
	void add( StreamTranscoder& streamTranscoder);

	/**
	 * @brief Initialize all added streams, processing codec latency.
	 * @note This can be called several times with no side effects.
	 * @note Can take a little bit of time.
	 */
	void preProcessCodecLatency();

	/**
	 * @brief Process all the streams, and ended the process depending on the transcode politic.
	 * @note The function manages all process: beginWrap(), preProcessCodecLatency(), process frames, and endWrap().
	 * @param progress: choose a progress, or create your own in C++ or in bindings by inherit IProgress class.
	 * @see IProgress
	 */
	void process( IProgress& progress );

	/**
	 * @param streamIndex: careful about the order of stream insertion of the Transcoder.
	 * @return a reference to a stream manage by the Transcoder.
	 */
	StreamTranscoder& getStreamTranscoder( size_t streamIndex ) const { return *_streamTranscoders.at( streamIndex ); }

	/**
	 * @brief Set the transcodage politic.
	 * @note By default eProcessMethodLongest.
	 * @param indexBasedStream: in case of process method eProcessMethodBasedOnStream, stop transcode at the end of the indicated stream.
	 */
	void setProcessMethod( const EProcessMethod eProcessMethod, const size_t indexBasedStream = 0 );

	/**
	 * @brief Set verbose mode for the Transcoder, its streams, and its output file.
	 * @note If you call it before adding the streams, no verbose mode will be set for the new streams.
	 * @note set av log level to AV_LOG_DEBUG
	 */
	void setVerbose( bool verbose = true );

private:
	void addRewrapStream( const std::string& filename, const size_t streamIndex );

	void addTranscodeStream( const std::string& filename, const size_t streamIndex, const size_t subStreamIndex, const size_t offset );  ///< Get profile from input
	void addTranscodeStream( const std::string& filename, const size_t streamIndex, const size_t subStreamIndex, ProfileLoader::Profile& profile, const size_t offset = 0 );

	void addDummyStream( const ProfileLoader::Profile& profile, const ICodec& codec );

	InputFile* addInputFile( const std::string& filename, const size_t streamIndex );

	/**
	 * @brief Get the duration of the stream.
	 */
	double getStreamDuration( size_t indexStream ) const;

	/**
	* @brief Get the duration of the shortest stream.
	* @note if there is only generated streams, return limit of double.
	*/
	double getMinTotalDuration() const;

	/**
	 * @brief Get the duration of the longest stream.
	 * @note if there is only generated streams, return limit of double.
	 */
	double getMaxTotalDuration() const;

	/**
	 * @brief Get the duration of the output program
	 * @note Depends on the streams, the process method, and the main stream index.
         */
	double getTotalDurationFromProcessMethod() const;

	/**
	 * @brief Set for each StreamTranscoder if it is an infinity stream (switch to generator at the end of the stream).
         */
	void manageInfinityStreamFromProcessMethod();

private:
	OutputFile&                      _outputFile;  ///< The output media file after process.
	std::vector< InputFile* >        _inputFiles;  ///< The list of input files which contain added streams.

	std::vector< StreamTranscoder* > _streamTranscoders;  ///< All streams of the output media file after process.
	std::vector< StreamTranscoder* > _streamTranscodersAllocated;  ///< Streams allocated inside the Transcoder.

	ProfileLoader _profileLoader;  ///< Objet to get existing profiles, and add new ones for the Transcoder.

	EProcessMethod _eProcessMethod;

	size_t _mainStreamIndex;  ///< Index of stream used to stop the process of transcode in case of eProcessMethodBasedOnStream.

	bool    _verbose;
};

}

#endif
