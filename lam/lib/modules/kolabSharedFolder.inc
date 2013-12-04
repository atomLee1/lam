<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
* Manages Kolab shared folders.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Kolab shared folders.
*
* @package modules
*/
class kolabSharedFolder extends baseModule { // TODO folder type
	
	/** cache for mailHost values */
	private $mailHostCache = null;
	
	/** list of folder types (label => id) */
	private $folderTypes = array();
	
	/**
	* Creates a new kolabSharedFolder object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->folderTypes = array(
			// TODO reactivate types when stable 3.1 is released
			/*_('Shared address book') => 'addressbook',
			_('Shared calendar') => 'calendar',
			_('Shared journal') => 'journal',
			_('Shared tasks') => 'task',*/
			_('Shared mail folder') => 'mail',
		);
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'kolab.png';
		// manages host accounts
		$return["account_types"] = array('kolabSharedFolderType');
		// alias name
		$return["alias"] = _("Kolab shared folder");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "normal");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=kolabSharedFolder)");
		// managed object classes
		$return['objectClasses'] = array('kolabSharedFolder', 'mailrecipient');
		// managed attributes
		$return['attributes'] = array('cn', 'kolabAllowSMTPRecipient', 'kolabAllowSMTPSender', 'kolabDeleteflag', 'acl',
			'alias', 'kolabDelegate', 'kolabFolderType', 'kolabTargetFolder', 'mailHost', 'mail');
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail server'), 'kolab_mailHost', null, 'mailHost'), true);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['kolab_mailHost'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['mailHost'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'kolab_mailHost' => 'mailHost',
		);
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _("Name"), 'attr' => 'cn',
				"Text" => _("Please enter a name for this folder.")
			),
			'mail' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The folder's email address.")
			),
			'kolabAllowSMTPRecipient' => array (
				"Headline" => _('Allowed recipients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
			),
			'kolabAllowSMTPRecipientList' => array (
				"Headline" => _('Allowed recipients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
							. ' ' . _("Multiple values are separated by semicolon.")
			),
			'kolabAllowSMTPSender' => array (
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
			),
			'kolabAllowSMTPSenderList' => array (
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
							. ' ' . _("Multiple values are separated by semicolon.")
			),
			'delegate' => array(
				"Headline" => _("Delegates"), 'attr' => 'kolabDelegate',
				"Text" => _("Delegates are allowed to act on behalf of the shared folder. This property is checked when using the Kolab smtp daemon (Postfix) to send emails.")
			),
			'delegateList' => array(
				"Headline" => _("Delegates"), 'attr' => 'kolabDelegate',
				"Text" => _("This is a comma separated list of delegates.")
			),
			'alias' => array(
				"Headline" => _("Email alias"), 'attr' => 'alias',
				"Text" => _("Email alias for this account.")
			),
			'aliasList' => array(
				"Headline" => _("Email alias list"), 'attr' => 'alias',
				"Text" => _("This is a comma separated list of eMail aliases.")
			),
			'mailHost' => array(
				"Headline" => _("Mailbox home server"), 'attr' => 'mailHost',
				"Text" => _("The name of the server where the mailbox is located.")
			),
			'kolabTargetFolder' => array(
				"Headline" => _("Target IMAP folder"), 'attr' => 'kolabTargetFolder',
				"Text" => _("The folder on the server where the shared folder is located (e.g. user/myfolder@example.com).")
			),
			'kolabFolderType' => array(
				"Headline" => _("Type"), 'attr' => 'kolabFolderType',
				"Text" => _("Specifies the folder type (e.g. shared mail folder).")
			),
			'deleteFlag' => array(
				"Headline" => _("Mark for deletion"), 'attr' => 'kolabDeleteflag',
				"Text" => _("This will set a special flag on the account which tells Kolabd to remove it. Use this to cleanly delete Kolab accounts (e.g. this removes mail boxes).")
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'kolabSharedFolder_cn',
				'description' => _('Name'),
				'help' => 'cn',
				'example' => 'folder',
				'required' => true
			),
			array(
				'name' => 'kolabSharedFolder_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('user@company.com'),
				'required' => true
			),
			array(
				'name' => 'kolabSharedFolder_mailHost',
				'description' => _('Mailbox home server'),
				'help' => 'mailHost',
				'example' => 'localhost',
			),
			array(
				'name' => 'kolabSharedFolder_kolabTargetFolder',
				'description' => _('Target IMAP folder'),
				'help' => 'kolabTargetFolder',
				'example' => 'user/myfolder@example.com',
				'required' => true
			),
			array(
				'name' => 'kolabSharedFolder_kolabFolderType',
				'description' => _('Type'),
				'help' => 'kolabFolderType',
				'example' => 'mail',
				'required' => true
			),
			array(
				'name' => 'kolabSharedFolder_kolabAllowSMTPRecipient',
				'description' => _('Allowed recipients'),
				'help' => 'kolabAllowSMTPRecipientList',
				'example' => '.com; -.net',
			),
			array(
				'name' => 'kolabSharedFolder_kolabAllowSMTPSender',
				'description' => _('Allowed senders'),
				'help' => 'kolabAllowSMTPSenderList',
				'example' => '.com; -.net',
			),
			array(
				'name' => 'kolabSharedFolder_aliases',
				'description' => _('Email aliases'),
				'help' => 'aliasList',
				'example' => 'user@domain,user2@domain'
			),
			array(
				'name' => 'kolabSharedFolder_delegates',
				'description' => _('Delegates'),
				'help' => 'delegateList',
				'example' => 'user@domain,user2@domain'
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Name'),
			'kolabAllowSMTPRecipient' => _('Allowed recipients'),
			'kolabAllowSMTPSender' => _('Allowed senders'),
			'aliases' => _('Email aliases'),
			'delegate' => _('Delegates'),
			'mailHost' => _('Mailbox home server'),
			'mail' => _('Email address'),
			'kolabTargetFolder' => _('Target IMAP folder'),
			'kolabFolderType' => _('Type'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['kolabAllowSMTPRecipient'][0] = array('ERROR', _('Allowed recipients'), _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPRecipient'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_kolabAllowSMTPRecipient', _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPSender'][0] = array('ERROR', _('Allowed senders'), _('Please enter a valid sender expression.'));
		$this->messages['kolabAllowSMTPSender'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_kolabAllowSMTPSender', _('Please enter a valid sender expression.'));
		$this->messages['extension'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_addExtension', _('You cannot add the Kolab group and shared folder extension at the same time.'));
		$this->messages['alias'][0] = array('ERROR', _('Email alias is invalid!'));  // third array value is set dynamically
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_aliases', _('Email alias list has invalid format!'));
		$this->messages['delegate'][0] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_delegates', _('Unknown delegate address: %s'));
		$this->messages['mailHost'][0] = array('ERROR', _('Mailbox home server name is invalid!'));  // third array value is set dynamically
		$this->messages['mailHost'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_mailHost', _('Mailbox home server name is invalid!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_mail', _('Please enter a valid email address!'));
		$this->messages['cn'][0] = array('ERROR', _('Name'), _('Please enter a name.'));
		$this->messages['kolabTargetFolder'][0] = array('ERROR', _('Target IMAP folder'), _('Please enter a target folder.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		// check if account is marked for deletion
		if (isset($this->attributes['kolabDeleteflag'])) {
			$container->addElement(new htmlOutputText(_('This account is marked for deletion.')));
			return $container;
		}
		$baseContainer = new htmlTable();
		// name
		$this->addSimpleInputTextField($baseContainer, 'cn', _('Name'), true);
		// mail
		$this->addSimpleInputTextField($baseContainer, 'mail', _('Email address'), true);
		// mailbox server
		if (!isset($this->orig['mailHost'][0])) {  // value currently not set
			$this->addSimpleInputTextField($baseContainer, 'mailHost', _('Mailbox home server'));
		}
		else {  // input is unchangable when set
			$baseContainer->addElement(new htmlOutputText(_('Mailbox home server')));
			$baseContainer->addElement(new htmlOutputText($this->attributes['mailHost'][0]));
			$baseContainer->addElement(new htmlHelpLink('mailHost'), true);
		}
		// target folder
		$this->addSimpleInputTextField($baseContainer, 'kolabTargetFolder', _('Target IMAP folder'), true);
		// folder type
		$possibleTypes = $this->folderTypes;
		$selectedTypes = array('mail');
		if (!empty($this->attributes['kolabFolderType'])) {
			$selectedTypes = $this->attributes['kolabFolderType'];
			if (!in_array($this->attributes['kolabFolderType'][0], $possibleTypes)) {
				$possibleTypes[$this->attributes['kolabFolderType'][0]] = $this->attributes['kolabFolderType'][0];
			}
		}
		$typeSelect = new htmlTableExtendedSelect('kolabFolderType', $possibleTypes, $selectedTypes, _('Type'), 'kolabFolderType');
		$typeSelect->setHasDescriptiveElements(true);
		$baseContainer->addElement($typeSelect, true);
		$baseContainer->addVerticalSpace('10px');
		// allowed recipients
		$this->addMultiValueInputTextField($baseContainer, 'kolabAllowSMTPRecipient', _('Allowed recipients'));
		// allowed senders
		$this->addMultiValueInputTextField($baseContainer, 'kolabAllowSMTPSender', _('Allowed senders'));
		$container->addElement($baseContainer, true);
		// mail aliases
		$container->addElement(new htmlSubTitle(_('Email aliases')), true);
		$this->addMultiValueInputTextField($container, 'alias');
		// delegates
		$delegates = searchLDAPByAttribute('mail', '*', 'inetOrgPerson', array('mail'), array('user'));
		for ($i = 0; $i < sizeof($delegates); $i++) {
			$delegates[$i] = $delegates[$i]['mail'][0];
		}
		sort($delegates);
		$container->addElement(new htmlSubTitle(_('Delegates')), true);
		$delegatesContainer = new htmlTable();
		$delegatesContainer->colspan = 3;
		if (isset($this->attributes['kolabDelegate'])) {
			for ($i = 0; $i < sizeof($this->attributes['kolabDelegate']); $i++) {
				$delegatesContainer->addElement(new htmlSelect('delegate' . $i, $delegates, array($this->attributes['kolabDelegate'][$i])));
				$delegatesContainer->addElement(new htmlButton('delDelegate' . $i, 'del.png', true));
				if ($i == 0) {
					$delegatesContainer->addElement(new htmlHelpLink('delegate'));
				}
				$delegatesContainer->addNewLine();
			}
		}
		// input box for new delegate
		$delegatesContainer->addElement(new htmlSelect('delegate', $delegates));
		$delegatesContainer->addElement(new htmlButton('addDelegate', 'add.png', true));
		if (empty($this->attributes['kolabDelegate'])) {
			$delegatesContainer->addElement(new htmlHelpLink('delegate'));
		}
		$container->addElement($delegatesContainer, true);
		// delete flag
		$this->loadMailHostCache();
		if (!$this->getAccountContainer()->isNewAccount && (sizeof($this->mailHostCache) > 0)) {
			$deleteContainer = new htmlTable();
			$deleteContainer->addElement(new htmlSpacer(null, '20px'), true);
			$deleteContainer->addElement(new htmlAccountPageButton(get_class($this), 'deleteFlag', 'open', _('Mark account for deletion')));
			$deleteContainer->addElement(new htmlHelpLink('deleteFlag'));
			$container->addElement($deleteContainer);
		}
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		if (empty($_POST['cn'])) {
			$errors[] = $this->messages['cn'][0];
		}
		// mail
		$this->attributes['mail'][0] = $_POST['mail'];
		if (empty($_POST['mail']) || !get_preg($_POST['mail'], 'email')) {
			$errors[] = $this->messages['mail'][0];
		}
		// mailbox server
		if (isset($_POST['mailHost'])) {
			if (($_POST['mailHost'] == "") && isset($this->attributes['mailHost'])) {
				unset($this->attributes['mailHost']);
			}
			elseif (!empty($_POST['mailHost'])) {
				if (get_preg($_POST['mailHost'], 'DNSname')) {
					$this->attributes['mailHost'][0] = $_POST['mailHost'];
				}
				else {
					$message = $this->messages['mailHost'][0];
					$message[] = $_POST['mailHost'];
					$errors[] = $message;
				}
			}
		}
		// target folder
		$this->attributes['kolabTargetFolder'][0] = $_POST['kolabTargetFolder'];
		if (empty($_POST['kolabTargetFolder'])) {
			$errors[] = $this->messages['kolabTargetFolder'][0];
		}
		// folder type
		$this->attributes['kolabFolderType'][0] = $_POST['kolabFolderType'];
		// allowed recipients
		$this->processMultiValueInputTextField('kolabAllowSMTPRecipient', $errors, 'kolabEmailPrefix');
		// allowed senders
		$this->processMultiValueInputTextField('kolabAllowSMTPSender', $errors, 'kolabEmailPrefix');
		// mail aliases
		$this->processMultiValueInputTextField('alias', $errors, 'email');
		// check old delegates
		$this->attributes['kolabDelegate'] = array();
		$i = 0;
		while (isset($_POST['delegate' . $i])) {
			if (isset($_POST['delDelegate' . $i])) {
				$i++;
				continue;
			}
			$this->attributes['kolabDelegate'][] = $_POST['delegate' . $i];
			$i++;
		}
		// check new delegate
		if (isset($_POST['addDelegate']) && ($_POST['delegate'] != "")) {
			$this->attributes['kolabDelegate'][] = $_POST['delegate'];
		}
		$this->attributes['kolabDelegate'] = array_unique($this->attributes['kolabDelegate']);
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to mark an account for deletion.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_deleteFlag() {
		$return = new htmlTable();
		$message = new htmlOutputText(_('Do you really want to mark this account for deletion?'));
		$return->addElement($message, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$serverTable = new htmlTable();
		$serverTable->addElement(new htmlTableExtendedSelect('deletionServer', $this->mailHostCache, array(), _('Server'), 'deleteFlag'));
		$return->addElement($serverTable, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttonGroup = new htmlGroup();
		$buttonGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'confirm', _('Mark account for deletion')));
		$buttonGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancel', _('Cancel')));
		$return->addElement($buttonGroup, true);
		return $return;
	}

	/**
	* Write variables into object and do some regex checks
	*/
	function process_deleteFlag() {
		if (isset($_POST['form_subpage_kolabSharedFolder_attributes_confirm'])) {
			// set delete flag
			$this->attributes['kolabDeleteflag'][0] = $_POST['deletionServer'];
		}
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		$delegates = searchLDAPByAttribute(null, null, 'inetOrgPerson', array('mail'), array('user'));
		for ($d = 0; $d < sizeof($delegates); $d++) {
			if (isset($delegates[$d]['mail'][0])) {
				$delegates[$d] = $delegates[$d]['mail'][0];
			}
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('kolabSharedFolder', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'kolabSharedFolder';
			}
			if (!in_array('mailrecipient', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'mailrecipient';
			}
			// cn
			$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['kolabSharedFolder_cn']];
			// mail
			if (!get_preg($rawAccounts[$i][$ids['kolabSharedFolder_mail']], 'email')) {
				$errMsg = $this->messages['mail'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			else {
				$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['kolabSharedFolder_mail']];
			}
			// mailbox server
			if (!empty($rawAccounts[$i][$ids['kolabSharedFolder_mailHost']])) {
				if (get_preg($rawAccounts[$i][$ids['kolabSharedFolder_mailHost']], 'DNSname')) {
					$partialAccounts[$i]['mailHost'] = $rawAccounts[$i][$ids['kolabSharedFolder_mailHost']];
				}
				else {
					$errMsg = $this->messages['mailHost'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// target folder
			$partialAccounts[$i]['kolabTargetFolder'] = $rawAccounts[$i][$ids['kolabSharedFolder_kolabTargetFolder']];
			// folder type
			$partialAccounts[$i]['kolabFolderType'] = $rawAccounts[$i][$ids['kolabSharedFolder_kolabFolderType']];
			// allowed recipients
			if (!empty($rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPRecipient']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPRecipient']]);
				for ($m = 0; $m < sizeof($mails); $m++) {
					if (get_preg($mails[$m], 'kolabEmailPrefix')) {
						$partialAccounts[$i]['kolabAllowSMTPRecipient'][] = $mails[$m];
					}
					else {
						$errMsg = $this->messages['kolabAllowSMTPRecipient'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
						break;
					}
				}
			}
			// allowed senders
			if (!empty($rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPSender']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPSender']]);
				for ($m = 0; $m < sizeof($mails); $m++) {
					if (get_preg($mails[$m], 'kolabEmailPrefix')) {
						$partialAccounts[$i]['kolabAllowSMTPSender'][] = $mails[$m];
					}
					else {
						$errMsg = $this->messages['kolabAllowSMTPSender'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
						break;
					}
				}
			}
			// add mail aliases
			if ($rawAccounts[$i][$ids['kolabSharedFolder_aliases']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['kolabSharedFolder_aliases']]);
				// check format
				for ($a = 0; $a < sizeof($aliases); $a++) {
					if (get_preg($aliases[$a], 'email')) {
						$partialAccounts[$i]['alias'][] = $aliases[$a];
					}
					// invalid format
					else {
						$errMsg = $this->messages['alias'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add delegates
			if ($rawAccounts[$i][$ids['kolabSharedFolder_delegates']] != "") {
				$newDelegates = explode(',', $rawAccounts[$i][$ids['kolabSharedFolder_delegates']]);
				// check format
				for ($d = 0; $d < sizeof($newDelegates); $d++) {
					if (in_array($newDelegates[$d], $delegates)) {
						$partialAccounts[$i]['kolabDelegate'][] = $newDelegates[$d];
					}
					// invalid format
					else {
						$errMsg = $this->messages['delegate'][0];
						array_push($errMsg, array($i, $newDelegates[$d]));
						$messages[] = $errMsg;
					}
				}
			}
		}
		return $messages;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Name'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'mailHost', _('Mailbox home server'));
		$this->addSimplePDFField($return, 'kolabTargetFolder', _('Target IMAP folder'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPRecipient', _('Allowed recipients'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPSender', _('Allowed senders'));
		$this->addSimplePDFField($return, 'aliases', _('Email aliases'), 'alias');
		$this->addSimplePDFField($return, 'delegate', _('Delegates'), 'kolabDelegate');
		if (!empty($this->attributes['kolabFolderType'])) {
			$type = $this->attributes['kolabFolderType'][0];
			$typeList = array_flip($this->folderTypes);
			if (isset($typeList[$type])) {
				$type = $typeList[$type];
			}
			$return[get_class($this) . '_kolabFolderType'] = array('<block><key>' . _('Type') . '</key><value>' . $type . '</value></block>');
		}
		return $return;
	}
	
	/**
	 * Loads the list of mail hosts into the cache.
	 */
	private function loadMailHostCache() {
		if ($this->mailHostCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(mailHost=*)', array('mailHost'), array('user'));
		$this->mailHostCache = array();
		foreach ($results as $result) {
			if (isset($result['mailhost'][0]) && !in_array_ignore_case($result['mailhost'][0], $this->mailHostCache)) {
				$this->mailHostCache[] = $result['mailhost'][0];
			}
		}
	}
	
}


?>