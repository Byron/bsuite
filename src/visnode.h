#ifndef PTEX_VISUALIZATION_NODE
#define PTEX_VISUALIZATION_NODE

#include <Ptexture.h>
#include <maya/MPxLocatorNode.h>

typedef PtexPtr<PtexFilter> PtexFilterPtr;
typedef PtexPtr<PtexTexture> PtexTexturePtr;

class PtexVisNode : public MPxLocatorNode
{
	public:
		PtexVisNode();
		virtual ~PtexVisNode();

		virtual MStatus compute(const MPlug&, MDataBlock&);
		virtual bool	setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx);
		virtual void    postConstructor();
		virtual void	draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus);

		static  void*   creator();
		static  MStatus initialize();

		static const MTypeId typeId;				//!< binary file type id
		static const MString typeName;				//!< node type name

	protected:
		//! \return filter type enumeration based on an int (in a safe manner as we do not cast)
		static PtexFilter::FilterType to_filter_type(int type);
		
		//! Assure a filter is present.
		//! \return false if this is not the case, usually because the filename is invalid
		bool assure_texture(MDataBlock& data);
		
		//! release the current texture (if there is one). This releases the filter as well !
		void release_texture_and_filter();
		
	protected:
		// Input attributes
		static MObject aPtexFileName;			//!< path to ptex file to use
		static MObject aPtexFilterType;			//!< type of filter for evaluation
		static MObject aPtexFilterSize;			//!< desired uv filter size
		static MObject aInMesh;					//!< mesh to display the ptex information for
		
		// output attributes
		static MObject aOutNumChannels;			//!< provide the number of channels in the file
		static MObject aOutNumFaces;			//!< amount of faces in the file
		static MObject aOutAlphaChannel;		//!< index of the channel providing alpha information
		static MObject aOutHasMipMaps;			//!< true if mipmaps are stored
		static MObject aOutHasEdits;			//!< true if edits are available
		static MObject aOutMetaDataKeys;		//!< string array of meta data keys
		static MObject aOutDataType;			//!< data type used in the channels
		static MObject aOutMeshType;			//!< mesh type used in file
		static MObject aOutUBorderMode;			//!< u border mode
		static MObject aOutVBorderMode;			//!< u border mode
		static MObject aNeedsCompute;			//!< dummy output (for now) to check if we need to compute
		

	protected:
		uint32_t		m_ptex_num_channels;	//!< Amount of channels currently stored in the file
		PtexFilterPtr	m_ptex_filter;			//!< filter ptr to be used for ptex evaluation
		PtexTexturePtr	m_ptex_texture;			//!< texture pointer
		MString			m_error;				//!< error string
};

#endif
