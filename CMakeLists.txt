file(GLOB HEADERS "include/graphene/upload_balances/*.hpp")

add_library( upload_balances
        upload_balances.cpp
           )

target_link_libraries( upload_balances graphene_chain graphene_app curl )
target_include_directories( upload_balances
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties(upload_balances.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   upload_balances

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/upload_balances" )

