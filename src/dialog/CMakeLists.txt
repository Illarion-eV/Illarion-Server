find_package( Luabind REQUIRED )

add_library( dialog INTERFACE )
target_sources( dialog 
    INTERFACE
        CraftingDialog.cpp
        Dialog.cpp
        InputDialog.cpp
        MerchantDialog.cpp
        MessageDialog.cpp
        SelectionDialog.cpp
)

target_link_libraries( dialog INTERFACE data )
target_link_libraries( dialog INTERFACE Luabind::Luabind )
target_include_directories( dialog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_compile_features( dialog INTERFACE cxx_std_17 )
